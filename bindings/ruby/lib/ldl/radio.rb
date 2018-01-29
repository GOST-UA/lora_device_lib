module LDL

    class Radio
    
        attr_accessor :buffer, :mode
    
        def initialize(broker)

            raise "SystemTime must be defined" unless defined? SystemTime
            
            @mode = :sleep
            @events = []  
            @rx = nil
            @tx = nil     
            @buffer = nil     
            @broker = broker 
            
        end
    
        def resetHardware
            SystemTime.wait(1)        
        end
    
        def transmit(data, **settings)            
            
            @broker.publish({
                :time => System.time,
                :tx_time => @mac.onAirTime(settings[:bw], settings[:sf], data.size),
                :data => data.dup,
                :sf => settings[:sf],
                :bw => settings[:bw],
                :cr => settings[:cr],
                :freq => settings[:freq],
                :power => settings[:power],                    
            }, "send")
            
            mac = @mac
            
            SystemTime.onTimeout(@mac.onAirTime(settings[:bw], settings[:sf], data.size)) do
            
                mac.io_event :tx_complete, SystemTime.time
                @broker.publish()

            end           
            
            true
        
        end
            
        def receive(**settings)
            
            broker = @broker
            mac = self
            
            # this is only accessed within the block below
            state = :listening
        
            rx_event = broker.subscribe "gateway_tx" do |msg,topic|            
            
                case msg[:type]
                when :rx_timeout
                
                    if state == :listening
                    
                        @mac.io_event :rx_timeout, System.time
                        broker.unsubscribe rx_event
                        
                    end
                       
                when :rx_start
                
                    case state
                    when :listening
                    
                        if msg[:sf] == settings[:sf] and msg[:bw] == settings[:bw] and msg[:freq] == settings[:freq]
                    
                            state = :rx
                            
                            SystemTime.onTimeout msg[:tx_time] do 
                            
                                broker.unsubscribe rx_event
                                @mac.io_event :rx_ready, System.time
                                
                            end
                            
                        end
                        
                    end
                
                else
                    raise
                end
                
            end
            
            SystemTime.onTimeout(settings[:interval]) do
            
                broker.publish({
            
                    :type => :rx_timeout
                
                }, "mac_#{mac.id}")
            
            end

            true
                    
        end
        
        def collect        
            buffer
        end
        
        def sleep
            mode = :sleep
        end
        
        def set_mac(mac)
            @mac = mac
        end
        
    end


end
