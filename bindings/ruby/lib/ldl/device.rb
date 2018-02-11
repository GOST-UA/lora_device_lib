module LDL

    class Device
    
        attr_reader :mac, :broker
    
        def initialize(broker, **opts)
        
            @running = false
            @broker = broker
            @mac = MAC.new(broker, **opts)
            
            @worker = Thread.new do
            
                broker.publish({:eui => devEUI}, "up")
            
                begin
                    yield(self)
                rescue Interrupt
                end
                
                broker.publish({:eui => devEUI}, "down")
            
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
                @worker.run
                @running = true
            end
            self
        end
        
        def stop            
            if running?
                @worker.raise Interrupt
                @worker.join
                @running = false
            end
            self
        end
    
    end

end
