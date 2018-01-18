module LDL

    # SystemTime is used as a constant only so that it can be  
    # referenced from within ExtMAC.
    #
    # It delegates a number of methods to a TimeSource instance.
    class SystemTime
    
        @source = nil
    
        # assign a time source
        # @param src [TimeSource]
        def source=(src)
            @source = src
            self
        end
    
        # @see TimeSource#time
        def self.time
            if @source
                @source.time
            else
                raise "need a time source"
            end
        end
        
        # @see TimeSource#onTimeout
        def self.onTimeout(interval, &block)
            if @source
                @source.onTimeout(interval, &block)
            else
                raise "need a time source"
            end
        end
        
        # @see TimeSource#wait
        def self.wait(interval)
            if @source
                @source.wait(interval)
            else
                raise "need a time source"
            end
        end
        
    end
    
end
