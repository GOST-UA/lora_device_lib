module LDL

    # a queue that supports waiting for on a timeout
    class TimeoutQueue
        
        attr_reader :max
        
        def initialize(**opt)
        
            @queue = []
            @mutex = Mutex.new
            @received = ConditionVariable.new
            
        end
        
        def push(event, **opt)
        
            with_mutex do
            
                if max.nil? or size < max
            
                    if opt[:head]
                        @queue.unshift event
                    else
                        @queue.push event
                    end
                    
                    @received.signal
                    
                end
                
                event
            
            end
        
        end
        
        def cancel(event)
        
            with_mutex do        
                @queue.delete(event)                                
            end
        
        end
        
        def pop(timeout=nil)
        
            if timeout
                        
                end_time = Time.now + timeout.to_f
            
            end
            
            with_mutex do
            
                while @queue.empty?
                
                    if timeout
                    
                        break unless ((time_now = Time.now) < end_time)
                    
                        @received.wait(@mutex, end_time - time_now)
                        
                    else
                    
                        @received.wait(@mutex)
                        
                    end
                
                end 
                
                raise ThreadError unless not @queue.empty?
                                       
                @queue.shift
                    
            end
        
        end

        def size        
            @queue.size        
        end
        
        def clear
            with_mutex do
                @queue.clear
            end
        end

        def with_mutex
            @mutex.synchronize do
                yield
            end
        end

        private :with_mutex
    
    end
    
end
