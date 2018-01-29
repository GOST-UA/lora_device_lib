module LDL

    class Radio
    
        attr_accessor :buffer, :mode, :broker, :mac
    
        def initialize(mac, broker)am

            raise "SystemTime must be defined" unless defined? SystemTime
            
            @mode = :sleep
            @events = []  
            @rx = nil
            @tx = nil     
            @buffer = nil     
            @broker = broker 
            @mac = mac
            
        end
    
        def resetHardware
            SystemTime.wait(1)        
        end
    
        def transmit(data, **settings)            
            
            @broker.publish({
                :time => System.time,
                :tx_time => mac.onAirTime(settings[:bw], settings[:sf], data.size),
                :data => data.dup,
                :sf => settings[:sf],
                :bw => settings[:bw],
                :cr => settings[:cr],
                :freq => settings[:freq],
                :power => settings[:power],                    
            }, "send")
            
            SystemTime.onTimeout(mac.onAirTime(settings[:bw], settings[:sf], data.size)) do
            
                mac.io_event :tx_complete, SystemTime.time
                
            end           
            
            true
        
        end
            
        def receive(**settings)
            
            broker = @broker
            
            # this is only accessed within the block below
            state = :listening
        
            rx_event = broker.subscribe eui.to_s do |msg,topic|            
            
                case msg[:type]
                when :rx_timeout
                
                    if state == :listening
                    
                        mac.io_event :rx_timeout, System.time
                        broker.unsubscribe rx_event
                        
                    end
                       
                when :rx_start
                
                    case state
                    when :listening
                    
                        if msg[:sf] == settings[:sf] and msg[:bw] == settings[:bw] and msg[:freq] == settings[:freq]
                    
                            state = :rx
                            
                            SystemTime.onTimeout msg[:tx_time] do 
                            
                                broker.unsubscribe rx_event
                                mac.io_event :rx_ready, System.time
                                
                            end
                            
                        end
                        
                    end
                
                else
                    raise
                end
                
            end
            
            SystemTime.onTimeout(settings[:interval]) do
            
                
            
            end

            true
                    
        end
        
        def collect        
            buffer
        end
        
        def sleep
            mode = :sleep
        end
        
    end


end
