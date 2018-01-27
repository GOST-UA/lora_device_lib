require 'socket'

module LDL

    class Gateway

        attr_reader :eui
        
        attr_reader :lati
        attr_reader :long
        attr_reader :alti
        attr_reader :rxnb
        attr_reader :rxok
        attr_reader :rxfw
        attr_reader :dwnb
        attr_reader :txnb
        attr_reader :ackr
        
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
            @ackr = 0   # percentage of upstream datagrams acked

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

            @tx_buffer = []

            @channel = []

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

                # receive radio messages
                rx_event = @broker.subscribe "device_tx" do |msg|

                    if chan = @channel.detect{|c|c[:freq] == msg[:freq]}

                        @rxnb += 1
                        @rxok += 1
                        @rxfw += 1

                        # todo rethink signal quality numbers
                        
                        
                        @q << {
                            :task => :upstream,
                            :rxpk => Semtech::RXPacket.new(
                                time: Time.now,
                                tmst: SystemTime.time & 0xffffffff,
                                freq: msg[:freq],
                                chan: @channel.index(chan),
                                rfch: 0,
                                stat: :ok,
                                modu: msg[:modu],                
                                datar: 0,
                                codr: 5,
                                rssi: -25,
                                lsnr: 8,
                                data: msg[:data]
                            )
                        }
                        
                    end

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

                        @txnb += 1
                        @token += 1

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
                        
                        @txnb += 1
                        @token += 1
                        
                        # recur on this interval
                        SystemTime.onTimeout (status_interval * TimeSource::INTERVAL) do

                            @q << {:task => :status}                

                        end
                        
                        s.write m.encode
                        
                    # send upstream
                    when :upstream

                        m = Semtech::PushData.new(token: @token, eui: eui, rxpk: job[:rxpk])
                        
                        @txnb += 1
                        @token += 1
                        
                        s.write m.encode

                    # message received from network
                    when :downstream

                        begin

                            msg = Semtech::Message.decode(job[:data])    


                            case msg.class
                            # network acks PullData (holepunch)
                            when Semtech::PullAck
                            
                            
                            # network acks PushData
                            when Semtech::PushAck

                                m = unacked.detect { |m| m.token ==  job[:msg] }

                            
                            # network wants to send over radio
                            when Semtech::PullResp
                            
                                
                            
                            else
                                puts "unexpected"
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

        def add_rx_channel(chIndex, freq, bw)
            @channel[chIndex] = {

                :freq => freq.to_i,
                :bw => bw,
            }
        end

        def with_mutex
            @mutex.synchronize do
                yield
            end
        end

        private :with_mutex, :add_rx_channel
    
    end

end
