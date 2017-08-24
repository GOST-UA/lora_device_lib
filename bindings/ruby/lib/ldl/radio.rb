module LDL

    class Radio
    
        def initialize(mac)
            super
            @mac = mac
            @rx = Queue.new
        end
    
        def resetHardware
            true
        end
    
        def transmit(data, **settings)
            Thread.new do
                sleep interval
                # do event
            end
            true
        end
        
        def receive
            Thread.new do
                sleep random_interval
                # do event
            end
            true
        end
        
        def collect
            true
        end
        
        def sleep
        end
        
        def input_message(msg)
            @rx << msg
        end
        
    end


end
