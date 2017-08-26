module LDL

    class Radio
    
        def initialize(fixture)
            @mode = :sleep
            @events = []  
            @rx = nil
            @tx = nil     
            @buffer = nil      
            @fixture = fixture           
            @mac = nil
        end
    
        def resetHardware
            0   # todo add the reset delay and work that into the eventing system
        end
    
        def transmit(data, **settings)            
            
            @mode = :tx
            @buffer = data.dup
            
            # what does it do?
            #
            # 1. At end of transmit time, it should pass entirety of message
            #    to the ???
            #
            # 2. It should pass the entirety of the message to the tx medium (which can work out if there is interference with other nodes)
            #
            
            @events << {
                :type => :tx_complete,
                :time => System.time + transmit_time
            }        
            
            # we need to know transmit time
            
            true
        end
        
        def receive(**settings)
            
            @events << {
                :type => :rx_timeout,
                :time => settings[:timeout] # this will be a symbol timeout so convert to ticks
            }
            
            @mode = :rx
            @rx = settings
                    
            true
                    
        end
        
        def collect        
            ""
        end
        
        def sleep
            @mode = :sleep
        end
        
        def tick        
            @events.select{|e|e[:time] >= SystemTime.time}.each do |e|
                @events.delete(e)                
                case e[:type]
                when :rx_timeout, :rx_ready, :tx_finished
                    if @mac
                        @mac.io_event(e[:type], SystemTime.time)
                    end 
                end                                
            end
        end
        
        # receive a message from the fixture
        def fixture_msg(msg)
            
        end
        
        def set_mac(mac)
            @mac = mac
        end
        
    end


end
