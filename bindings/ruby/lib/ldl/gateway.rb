require 'socket'

module LDL

    class Gateway
    
        MAX_TOKENS = 100
        
        # maximum allowable advance send scheduling
        MAX_ADVANCE_TIME = 10
        
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
        
        attr_reader :broker
        
        def increment_token
            @token += 1
            self
        end
        
        def increment_txnb
            @txnb += 1
            self
        end
        
        def increment_dwnb
            @dwnb += 1
            self
        end
        
        def increment_rxok
            @rxok += 1
        end
        
        def increment_rxnb
            @rxnb += 1
        end
        
        def increment_rxfw
            @rxfw += 1
        end
        
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

            raise TypeError unless broker.kind_of? Broker
            raise TypeError unless eui.kind_of? EUI64

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
                @keepalive_interval = 20
            end
            
            if opts.has_key? :status_interval
                @status_interval = opts[:status_interval].to_i
            else
                @status_interval = 20
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
                # ttn not responding to this, makes our qos look bad
                #@q << {:task => :status}

                buffer = []

                tx_begin = broker.subscribe "tx_begin" do |m1|
                
                    # don't listen to our own stuff
                    if m1[:eui] != eui                    
                        buffer.push m1
                    end
                    
                end

                tx_end = broker.subscribe "tx_end" do |m2|

                    if m1 = buffer.detect{ |v| v[:eui] == m2[:eui] }

                        increment_rxnb
                        increment_rxok
                        increment_rxfw

                        # todo rethink signal quality numbers
                        
                        @q << {
                            :task => :upstream,
                            :rxpk => Semtech::RXPacket.new(
                                tmst: tmst,
                                freq: m1[:freq].to_f / 1000000.0,
                                data: m1[:data],
                                datr: "SF#{m1[:sf]}BW#{m1[:bw]/1000}",                                                        
                                rssi: -80,
                                lsnr: 9,
                                chan: m1[:channel],
                                rfch: 1
                                
                            )
                        }
                        
                        buffer.delete m1
                        
                    end

                end
                
                s = UDPSocket.new
                s.connect(host, port)
                
                Thread.new do
                
                    begin

                        loop do
                        
                            reply = s.recvfrom(1024, 0)
                            @q << {:task => :downstream, :data => reply.first}

                        end

                    rescue
                    end
                
                end.run
                
                broker.publish({:eui => eui}, 'up')

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
        
                        increment_token
                        increment_txnb
                        
                        # schedule keep alive: note this will probably fire once after we stop this thread
                        SystemTime.onTimeout (keepalive_interval * TimeSource::INTERVAL) do

                            @q << {:task => :keepalive}                

                        end

                        puts "send keep alive:"
                        puts m.inspect
                        puts ""

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
                                ackr: ackr.round(1),
                                dwnb: dwnb,
                                txnb: txnb
                            )
                        )
                        
                        add_token @token
                        
                        increment_token
                        increment_txnb
                        
                        # recur on this interval
                        SystemTime.onTimeout (status_interval * TimeSource::INTERVAL) do

                            @q << {:task => :status}                

                        end
                        
                        puts "send status:"
                        puts m.inspect
                        puts ""
                        
                        s.write m.encode
                        
                    # send upstream
                    when :upstream

                        increment_rxnb
                        increment_rxok
                        increment_rxfw
                        
                        m = Semtech::PushData.new(token: @token, eui: eui, rxpk: [job[:rxpk]])
                        
                        add_token @token
                        
                        increment_token
                        increment_txnb

                        puts "sending upstream:"
                        puts m.inspect
                        puts ""
                        
                        puts "upstream rxpk:"
                        puts job[:rxpk].to_json
                        puts ""
                        
                        s.write m.encode
         
                    # message received from network
                    when :downstream

                        begin

                            msg = Semtech::Message.decode(job[:data])    

                            puts "message received:"
                            puts msg.inspect
                            puts ""

                            case msg.class
                            when Semtech::PushAck, Semtech::PullAck    
                                ack_token(msg.token)                              
                            when Semtech::PullResp
                            
                                increment_dwnb

                                SystemTime.onTimeout 0 do

                                    # send now
                                    if msg.txpk.imme
                                    
                                        s.write(
                                            Semtech::TXAck.new(
                                                token: msg.token,
                                                eui: eui,
                                                txpk_ack: Semtech::TXPacketAck.new(error: 'NONE')
                                            ).encode
                                        )
                
                                        increment_token
                                        increment_txnb
                                        
                                        transmit(msg.txpk)
                                                                
                                    # send according to time stamp
                                    elsif msg.txpk.tmst
                                
                                        timeNow = tmst
                                    
                                        puts "tmst: #{timeNow}"
                                        puts "msg_tmst: #{msg.txpk.tmst}"
                                        
                                        if msg.txpk.tmst > timeNow
                                            delta = msg.txpk.tmst - timeNow
                                        else
                                            delta = timeNow - msg.txpk.tmst
                                        end
                                    
                                        puts "delta: #{delta}"
                                        
                                        # I expect the advance time to never be more than tens of seconds
                                        if delta <= 10000000
                                        
                                            puts "need to send in #{delta/10} ticks (SystemTime.time will be #{SystemTime.time + (delta/10)})"
                                            puts ""
                                    
                                            s.write(
                                                Semtech::TXAck.new(
                                                    token: msg.token,
                                                    eui: eui,
                                                    txpk_ack: Semtech::TXPacketAck.new(error: 'NONE')
                                                ).encode
                                            )
                
                                            increment_token
                                            increment_txnb
                                            
                                            SystemTime.onTimeout((delta/10) + 1) do
                                            
                                                transmit(msg.txpk)
                                            
                                            end
                                        
                                        else
                                    
                                            puts "too late to send"
                                            puts ""
                                        
                                            s.write(
                                                Semtech::TXAck.new(
                                                    token: msg.token,
                                                    eui: eui,
                                                    txpk_ack: Semtech::TXPacketAck.new(error: 'TOO_LATE')
                                                ).encode
                                            )
                    
                                            increment_token
                                            increment_txnb
                                            
                                        end
                                    
                                    # don't support the use of time field 
                                    elsif msg.tkpk.time
                                    
                                        s.write(
                                            Semtech::TXAck.new(
                                                token: msg.token,
                                                eui: eui,
                                                txpk_ack: Semtech::TXPacketAck.new(error: 'GPS_UNLOCKED')
                                            ).encode
                                        )
                                        
                                        increment_token
                                        increment_txnb
                                    
                                    end
                                
                                end
                            
                            end

                        rescue => e
                        
                            puts job[:data]
                        
                            puts e
                            puts e.backtrace.join("\n")
                        
                        
                        end

                    end

                end

                broker.unsubscribe tx_begin
                broker.unsubscribe tx_end
                
                broker.publish({:eui => eui}, 'down')

            end

        end

        def running?
            @running
        end

        def transmit(txpk)
                  
            puts "gateway is transmitting at #{SystemTime.time}"
            puts ""
                        
            m1 = {
                :eui => eui,    # eui of the sender
                :time => SystemTime.time,
                :data => txpk.data,                            
                :sf => txpk.sf,
                :bw => txpk.bw,
                :cr => txpk.codr,
                :freq => (txpk.freq * 1000000).to_i,
                :power => 0                            
            }
            
            m2 = {
                :eui => m1[:eui]                        
            }
            
            broker.publish m1, "tx_begin"
            
            SystemTime.onTimeout MAC.transmitTimeDown(txpk.bw, txpk.sf, txpk.data.size) do 
                
                broker.publish m2, "tx_end"
                
            end
                
            
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
                if t[0] == token                
                    t[1] = true                    
                end                
            end
            self
        end
        
        def ackr
            (@tokens.select{|t|t.last}.size.to_f / @tokens.size.to_f ) * 100.0
        end
        
        def tmst
            ( (SystemTime.time * 10) & 0xffffffff )
        end
        
        private :with_mutex, :add_token, :ack_token, :tmst, :transmit
    
    end

end
