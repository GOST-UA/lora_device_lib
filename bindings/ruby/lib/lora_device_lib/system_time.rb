module LoraDeviceLib

    class SystemTime
    
        @time = 0
    
        def self.time
            @time
        end
        
        def self.time=(usec)
            @time = usec
        end
        
        def increment(usec)            
            @time += usec
        end
    
    end
    
end
