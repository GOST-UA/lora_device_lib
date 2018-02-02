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
            
            @txRate = 4
            @txPower = 0
            
            this = self
            
            @ticker = Proc.new do
                this.tick
                if this.timeUntilNextEvent
                    SystemTime.onTimeout this.timeUntilNextEvent, &ticker
                end
            end
            
        end
        
        attr_reader :devEUI, :appEUI
        
        def io_event(event, time)
        
            Logger.debug "io_event #{event} at #{time}"
        
            super
            ticker.call
        end
        
        # perform a join
        def join            
            rq = Queue.new
            this = self
            SystemTime.onTimeout 0 do
            
                this.method(:join).super_method.call do |result|
                    rq << result
                end            
                this.ticker.call
            end
            raise JoinTimeout.new "timeout waiting for join confirmation" unless rq.pop == :join_success
            self
        end
    
        # send data
        #
        # @param port [Integer]
        # @param data [String]
        # @param opts [Hash]
        #
        # @option opts [true,false] :confirmed 
        def send(port, data, **opts)        
            rq = Queue.new
            this = self
            SystemTime.onTimeout 0 do
                this.method(:send).super_method.call(port, data, **opts) do |result|
                    rq << result
                end
                this.ticker.call
            end            
            raise SendTimeout unless rq.pop == :tx_complete
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
            rq = Queue.new
            SystemTime.onTimeout 0 do
                @rx_handler = block
                rq << nil
            end
            rq.pop
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
        
        # @private
        def with_mutex
            @mutex.synchronize do
                yield
            end
        end
    
        private :with_mutex
    
    end

end
