module LDL

    # A TimeSource instance provides #onTimeout and #wait methods,
    # as well as #time method for returning system time.
    #
    # The timer queue ensures that events can be scheduled to occur when system time
    # is a specific value. Events are never late or early unless they have been programmed to be that way, 
    # regardless of when the garbage collector runs.
    # 
    class TimeSource
    
        INTERVAL = MAC::TIMEBASE.to_f
    
        # @return [Integer] system time
        attr_reader :time
        
        def running?
            @running
        end
        
        def initialize        
        
            @running = false
            @mutex = Mutex.new            
            @time = 0      
            prevTime = Time.now              
            @update = TimeoutQueue.new

            @queue = []

            @ticker = Thread.new do             
            
                loop do
                        
                    begin
                    
                        if @queue.empty?
                            self.onTimeout INTERVAL do
                            end                    
                        end
                        
                        break unless @update.pop( to_sec(@queue.first[:interval]) )
                        
                    rescue ThreadError
                    end

                    loop do

                        # work out time that has passed since last tick
                        timeNow = Time.now                    
                        delta = to_ticks(timeNow - prevTime)
                        prevTime = timeNow
                        
                        serviced = 0
                        
                        loop do
                        
                            event = nil
                        
                            with_mutex do
                            
                                if not @queue.empty? 
                                
                                    if @queue.first[:interval] <= delta
                                        event = @queue.shift
                                    else
                                        @queue.first[:interval] -= delta
                                    end
                                end                                                        
                            end
                        
                            if event
                        
                                delta -= event[:interval]
                                @time += event[:interval].to_i
                                event[:block].call
                                
                                serviced += 1
                                
                            else
                            
                                @time += delta.to_i
                                break
                                
                            end
                               
                        end
                        
                        break unless serviced > 0
                        
                    end
                        
                end
            
            end
    
        end
        
        # schedule a block to be run after an interval
        #
        # @param interval [Integer]
        # @param block [Proc] code to run
        #
        # @return [Hash] scheduled event record
        def onTimeout(interval, &block)
        
            raise ArgumentError unless interval.kind_of? Numeric
            
            ref = {:interval => interval.to_i, :block => block}
            
            with_mutex do                
                if @queue.empty?
                    @queue << ref
                else                
                    @queue.each.with_index do |v, i|
                        if v[:interval] >= interval
                            v[:interval] -= interval
                            @queue.insert(i, ref)                                                    
                            break
                        else
                            ref[:interval] -= v[:interval]                            
                            if @queue.last == v
                                @queue << ref
                                break
                            end
                        end
                    end
                end                
                @update.push ref                
            end
            
            ref
        end
        
        def cancel(ref)
            with_mutex do
                @queue.each.with_index do |v,i|
                    if v == ref                        
                        if v != @queue.last
                            @queue[i+1][:interval] += v[:interval]
                        end
                        @queue.delete(v)
                    end
                end
            
                @queue.delete(ref)            
            end
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
