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
            @time = 0      
            base = 0
            prevTime = Time.now              
            @update = TimeoutQueue.new

            @ticker = Thread.new do             
            
                queue = []

                loop do
                        
                    begin

                        event = @update.pop( queue.empty? ? nil : to_sec(queue.first[:interval]) )

                        if event.nil?
                            break
                        end
                        
                    rescue ThreadError
                    
                        event = nil

                    end

                    if event.kind_of? Hash
                    
                        case event[:op]
                        when :add

                            queue << event
                            queue.sort_by! { |e| e[:interval] }
                            
                        when :remove
                        
                            queue.delete( event[:ref] )
                            
                        else
                            raise
                        end
                    
                    end
            
                    timeNow = Time.now
                    
                    delta = to_ticks(timeNow - prevTime)

                    prevTime = timeNow
                    
                    queue = queue.map do |v|
                    
                        if v[:interval] <= delta
                        
                            @time = base + v[:interval]

                            v[:block].call
                            nil
                            
                        else
                        
                            v[:interval] -= delta
                            v
                            
                        end
                        
                    end.compact
                    
                    base += delta
                    @time = base
                    
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
        
        def cancel(ref)
            @update.push({
                :op => :remove,
                :cb => ref
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
            with_mutex do
                if not running?
                    @ticker.run
                end
                self
            end
        end
        
        # stop the time source
        #
        def stop
            with_mutex do
                if running?
                    @update.push(nil, :head => true)
                    @ticker.join
                end        
                self
            end
        end
        
        def with_mutex
            @mutex.synchronize do
                yield
            end
        end

        INTERVAL = 100000.0

        # convert 20us tick interval to seconds
        def to_sec(interval)
            interval / INTERVAL
        end

        # convert seconds to 20us ticks
        def to_ticks(sec)
            (sec * INTERVAL).to_i
        end
        
        private :with_mutex, :to_sec, :to_ticks
    
    end

end
