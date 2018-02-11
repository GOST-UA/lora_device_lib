require 'ldl/ext_mac'

module LDL

    # extends ExtMAC to add threadsafe interfaces
    class MAC < ExtMAC        

        attr_reader :ticker
        
        # @param broker [Broker] for sending/recieving events
        # @param opts [Hash]
        #
        # @option opts [Symbol] :region
        #
        def initialize(broker, **opts)
        
            raise "SystemTime must be defined" unless defined? SystemTime
            
            @mutex = Mutex.new            
            
            super(Radio.new(self, broker), **opts)
            
            this = self
            
            @ticker = Proc.new do
                this.tick
                
                if this.ticksUntilNextEvent                
                    SystemTime.onTimeout this.ticksUntilNextEvent, &ticker
                end
                
            end
            
        end
        
        attr_reader :devEUI, :appEUI
        
        def tick            
            with_mutex do
                super                
            end
            self
        end
        
        # @return [Integer]
        def ticksUntilNextChannel
            with_mutex do
                super
            end            
        end
        
        def io_event(event, time)
        
            #Logger.debug "io_event #{event} at #{time}"
        
            super
            ticker.call
        end
        
        # perform a join
        #
        # @note blocking call
        #
        def join            
        
            rq = Queue.new
            with_mutex do
                super do |result|            
                    rq << result
                end                                            
            end
            
            this = self
            
            SystemTime.onTimeout 0 do 
                this.ticker.call
            end
            
            raise JoinTimeout.new "timeout waiting for join confirmation" unless rq.pop == :ready
            self
        end
    
        # send data
        #
        # @note blocking call
        #
        # @param port [Integer]
        # @param data [String]
        # @param opts [Hash]
        #
        # @option opts [true,false] :confirmed 
        def data(port, data, **opts)        
            rq = Queue.new
            with_mutex do
                super(port, data, **opts) do |result|
                    rq << result
                end
            end
            ticker.call
            raise SendTimeout unless rq.pop == :ready
            self            
        end
        
        # action to perform when downstream data is received
        #
        # @param block [Proc] 
        #
        # @example 
        #   mac.on_receive do |port, data|
        #      puts "received #{data} on port #{port}"
        #   end
        #
        def on_receive(&block)
            with_mutex do
                @rx_handle = block
            end        
            self
        end
        
        # set a rate
        #
        # @param value [Integer]
        def rate=(value)
            with_mutex do
                super
            end
            self
        end
        
        # get the current rate
        # @return [Integer]
        attr_reader :rate
    
        # set a power setting
        #
        # @param value [Integer]
        def power=(value)
            with_mutex do
                super
            end
            self
        end
        
        # get the current power setting
        # @return [Integer]
        attr_reader :power
        
        def with_mutex
            @mutex.synchronize do
                yield
            end
        end
    
        private :with_mutex
    
    end

end
