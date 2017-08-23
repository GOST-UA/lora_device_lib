module LDL

    class Radio
    
        def initialize(mac)
            @mac = mac
            @rx = Queue.new
        end
    
        def resetHardware
        end
    
        def transmit(**settings)
            Thread.new do
                sleep interval
                # do event
            end
        end
        
        def receive
            Thread.new do
                sleep random_interval
                # do event
            end
        end
        
        def collect            
        end
        
        def random
            rand(0..255)
        end
        
        def sleep
        end
        
        def input_message(msg)
            @rx << msg
        end
        
    end


end
