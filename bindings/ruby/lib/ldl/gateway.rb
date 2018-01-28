require 'socket'

module LDL

    class Gateway
    
        MAX_TOKENS = 100
        
        # maximum allowable advance send scheduling
        MAX_ADVANCE_TIME = 
        
        attr_reader :eui
        
        # GPS latitude of the gateway in degree (float, N is +)
        attr_reader :lati 
        
        # GPS latitude of the gateway in degree (float, E is +)
        attr_reader :long 
        
        # GPS altitude of the gateway in meter RX (integer)
        attr_reader :alti 
        
        # Number of radio packets received (unsigned integer)
        attr_reader :rxnb 
        
        # Number of radio packets received with a valid PHY CRC
        attr_reader :rxok 
        
        # Number of radio packets forwarded (unsigned integer)
        attr_reader :rxfw 
        
        # Number of downlink datagrams received (unsigned integer)
        attr_reader :dwnb 
        
        # Number of packets emitted (unsigned integer)
        attr_reader :txnb 
        
        attr_reader :host
        attr_reader :port
        
        # @return [Integer] seconds
        attr_reader :keepalive_interval
        
        # @return [Integer] seconds
        attr_reader :status_interval
        
        # @param broker [Broker]
        # @param eui [EUI64] gateway identifier
        # @param opts [Hash]
        #
        # @option opts [Float] :long longitude
        # @option opts [Float] :lati latitude
        # @option opts [Integer] :alti altitude
        #
        # @option opts [String] :host upstream hostname
        # @option opts [Integer] :port upstream port
        #
        # @option opts [Array<Hash>]
        #
        # @option opts [Integer] :keepalive pull-data interval (seconds)
        #
        def initialize(broker, eui, **opts)            

            raise "SystemTime must be defined" unless defined? SystemTime

            @eui = eui
            @broker = broker
            
            @running = false

            # stats
            @rxnb = 0   # radio packets received 
            @rxok = 0   # radio packets received (with OK CRC)
            @rxfw = 0   # radio packets forwarded (upstream?)            
            @dwnb = 0   # number of downlink datagrams received
            @txnb = 0   # number of datagrams emmitted
            
            @tokens = []            
            
            # use as message token, increment for each message sent
            @token = rand 0..0xffff

            if opts.has_key? :lati
                @lati = opts[:lati].to_f
            else
                @lati = 51.4576
            end

            if opts.has_key? :long
                @long = opts[:long].to_f
            else
                @long = 0.9705
            end

            if opts.has_key? :alti
                @alti = opts[:alti].to_i
            else
                @alti = 61
            end

            if opts.has_key? :port
                @port = opts[:port]
            else
                @port = 1700
            end

            if opts.has_key? :host
                @host = opts[:host]
            else
                @host = "router.eu.thethings.network"
            end

            if opts.has_key? :keepalive_interval
                @keepalive_interval = opts[:keepalive_interval].to_i
            else
                @keepalive_interval = 10
            end
            
            if opts.has_key? :status_interval
                @status_interval = opts[:status_interval].to_i
            else
                @status_interval = 10
            end

            @q = Queue.new

            @t = []

            # worker thread
            @t << Thread.new do 

                # nothing old
                @q.clear

                # start the keep alive push
                @q << {:task => :keepalive}
                
                # start the status push
                @q << {:task => :status}

                # receive radio messages on all channels ;)
                rx_event = @broker.subscribe "device_tx" do |msg|

                    raise ArgumentError unless msg.kind_of? Hash
                    raise ArgumentError unless msg.has_key? :freq
                    raise ArgumentError unless msg.has_key? :data
                    raise ArgumentError unless msg.has_key? :sf
                    raise ArgumentError unless msg.has_key? :bw

                    @rxnb += 1
                    @rxok += 1
                    @rxfw += 1

                    # todo rethink signal quality numbers
                    
                    @q << {
                        :task => :upstream,
                        :rxpk => Semtech::RXPacket.new(
                            tmst: tmst,
                            freq: msg[:freq],
                            data: msg[:data],
                            datr: "SF#{msg[:sf]}BW#{msg[:bw]}",                                                        
                        )
                    }

                end
                
                s = UDPSocket.new
                s.connect(host, port)
                
                rx = Thread.new do
                
                    begin

                        loop do

                            reply, from = s.recvfrom(1024, 0)
                            @q << {:task => :downstream, :data => reply}

                        end

                    rescue
                    end
                
                end.run

                loop do

                    job = @q.pop
                    
                    case job[:task]
                    # stop the gateway by closing the socket and breaking this loop
                    when :stop

                        s.close # will cause exception for rx thread waiting on socket
                        break

                    # perform a keep alive
                    when :keepalive

                        m = Semtech::PullData.new(token: @token, eui: eui)
                        
                        add_token @token
                        
                        @token += 1 # advance our token
                        @txnb += 1  # number of packets emitted

                        # schedule keep alive: note this will probably fire once after we stop this thread
                        SystemTime.onTimeout (keepalive_interval * TimeSource::INTERVAL) do

                            @q << {:task => :keepalive}                

                        end

                        s.write m.encode

                    # send status upstream
                    when :status
                    
                        m = Semtech::PushData.new(
                            token: @token, 
                            eui: eui, 
                            stat: Semtech::StatusPacket.new(
                                lati: lati,
                                long: long,
                                alti: alti,
                                rxnb: rxnb,
                                rxok: rxok,
                                rxfw: rxfw,
                                ackr: ackr,
                                dwnb: dwnb,
                                txnb: txnb
                            )
                        )
                        
                        add_token @token
                        
                        @token += 1 # advance our token
                        @txnb += 1  # number of packets emitted
                        
                        # recur on this interval
                        SystemTime.onTimeout (status_interval * TimeSource::INTERVAL) do

                            @q << {:task => :status}                

                        end
                        
                        s.write m.encode
                        
                    # send upstream
                    when :upstream

                        @rxnb += 1  # number radio packets received
                        @rxok += 1  # number radio packets received (with ok CRC)
                        @rxfw += 1  # number radio packets forwarded
                        
                        m = Semtech::PushData.new(token: @token, eui: eui, rxpk: [job[:rxpk]])
                        
                        add_token @token
                        
                        @token += 1 # advance our token
                        @txnb += 1  # number of packets emitted
                        
                        s.write m.encode

                    # actually send on the radio
                    when :tx
                    
                        m = Semtech::TXAck.new(
                            token: msg.token,
                            eui: eui,
                            txpk_ack: Semtech::TXPacketAck.new
                        )
                        
                        @token += 1 # advance our token
                        @txnb += 1  # number of packets emitted
                        
                        s.write m.encode
                            
                    # message received from network
                    when :downstream

                        begin

                            msg = Semtech::Message.decode(job[:data])    

                            case msg.class
                            when Semtech::PushAck, Semtech::PullAck                                
                                ack_token msg.token                                
                            when Semtech::PullResp

                                @dwnb += 1  # number of downlink datagrams received

                                # send now
                                if msg.txpk.imme
                                    
                                    s.write(tx_ack('NONE'))
                                    
                                    @q << {:task => :tx, :txpk => msg.txpk}                
                                                                
                                # send according to last timestamp
                                elsif not msg.txpk.tmst.nil?
                                
                                    
                                # don't support the use of time field 
                                elsif msg.tkpk.time
                                
                                    s.write(tx_ack('GPS_UNLOCKED'))
                                
                                end
                            
                            end

                        rescue
                        end

                    end

                end

                @broker.unsubscribe rx_event

            end

        end

        def running?
            @running
        end

        # start worker
        def start
            if not running?
                @t.each(&:run)
                @running = true
            end
            self
        end

        # stop worker
        def stop
            if running?
                @q << {:task => :stop}
                @t.each(&:join)
                @running = false
            end
            self
        end

        def with_mutex
            @mutex.synchronize do
                yield
            end
        end

        def add_token(token)            
            if @tokens.size == MAX_TOKENS
                @tokens.pop                
            end            
            @tokens.unshift([token,false])
            self
        end
        
        def ack_token(token)
            @tokens.each do |t|
                if t.first == token
                    t.last = true
                end
            end
            self
        end
        
        def ackr
            (@tokens.select{|t|t.last}.size.to_f / @tokens.size.to_f ) * 100.0
        end
        
        def tmst
            (SystemTime.time * 20) & 0xffffffff
        end
        
        def tx_ack(error)
            
            m = Semtech::TXAck.new(
                token: msg.token,
                eui: eui,
                txpk_ack: Semtech::TXPacketAck.new
            )
            
            @token += 1 # advance our token
            @txnb += 1  # number of packets emitted
            
            
            
        end
        
        private :with_mutex, :add_token, :ack_token, :tmst, :tx_ack
    
    end

end
