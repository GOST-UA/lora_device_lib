module LDL

    class Device
    
        attr_reader :mac
    
        def initialize(broker, **opts)
        
            @running = false
            @broker = broker
            @mac = Mac.new(**opts)
            
            @worker = Thread.new do
            
                yield
            
            end
            
        end
        
        def devEUI
            mac.send __method__
        end
        def appEUI
            mac.send __method__
        end
        
        def running?
            @running
        end
        
        def start
            if not running?
                
            end
            self
        end
        
        def stop            
            if running?
                
            end
            self
        end
    
    end

end
