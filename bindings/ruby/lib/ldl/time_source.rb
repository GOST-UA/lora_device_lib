module LDL

    # A TimeSource instance provides #onTimeout and #wait methods,
    # as well as #time method for returning system time.
    #
    # The timer queue ensures that events can be scheduled to occur when system time
    # is a specific value. Events are never late or early unless they have been programmed to be that way, 
    # regardless of when the garbage collector runs.
    # 
    class TimeSource
    
        # @return [Integer] system time
        attr_reader :time
        
        def running?
            @running
        end
        
        def initialize        
        
            @running = false
            @mutex = Mutex.new
            @queue = []
            @time = 0      
            @base = 0
            @prevTime = Time.now              
            @update = TimeoutQueue.new
           
            @ticker = Thread.new do             
            
                loop do
                        
                    begin
                        
                        event = update.pop( @queue.empty? ? nil : @queue.first[:interval] )
                        
                        if event.nil?
                            break
                        end
                        
                    rescue ThreadError
                    
                        event = nil
                        
                    end
            
                    if event.kind_of? Hash
                    
                        case event[:op]
                        when :add
                            @queue << event
                            @queue.sort_by! { |e| e[:interval] }
                        else
                            raise
                        end
                    
                    end
            
                    timeNow = Time.now
                    
                    delta = timeNow - @prevTime
                    
                    @prevTime = timeNow
                        
                    @queue = @queue.map do |v|
                    
                        if v[:interval] <= delta
                        
                            @time = @base + v[:interval]
                            v[:block].call
                            nil
                            
                        else
                        
                            v[:interval] -= delta
                            v
                            
                        end
                        
                    end.compact
                    
                    @base += delta
                    @time = @base
                    
                end
            
            end
    
        end
        
        # schedule a block to be run after an interval
        #
        # - non-blocking
        #
        # @param interval [Integer]
        # @param block [Proc] code to run
        #
        # @return [Hash] scheduled event record
        def onTimeout(interval, &block)
            @update.push({
                :op => :add,
                :interval => interval,
                :block => block,
                :thread => Thread.current
            })
        end
        
        # wait (block) for a time interval
        #
        # @param interval [Interval]
        def wait(interval)
            wq = Queue.new
            onTimeout(interval) do
                wq.push nil
            end
            wq.pop
        end
        
        # start the time source
        #
        def start
            if not running?
                @ticker.run
            end
            self
        end
        
        # stop the time source
        #
        def stop
            if running?
                @update.push(nil, :head => true)
                @ticker.join
            end        
            self
        end
        
        def with_mutex
            @mutex.synchronize do
                yield
            end
        end
        
        private :with_mutex
    
    end

end
