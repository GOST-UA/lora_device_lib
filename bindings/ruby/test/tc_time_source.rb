require 'minitest/autorun'
require 'ldl'
require 'timeout'

class TestTimeSource < Minitest::Test

    include LDL

    attr_reader :state

    def setup
        Thread.abort_on_exception = true
        @state = TimeSource.new
        state.start
    end

    def test_wait

        Timeout::timeout 1 do
            state.wait 9999
        end

    end

    def test_onTimeout

        q = Queue.new
    
        state.onTimeout 999 do
        
            # #time must be predictable
            assert_equal 999, state.time

            q.push nil
        
        end

        Timeout::timeout 1 do
            q.pop
        end
        
    end
    
    def info(time1, time2, time3)
        #puts "start: #{time1}, now: #{time2}, expected: #{time3}"         
    end
    
    def test_multiple_onTimeout
    
        q = Queue.new
    
        state.onTimeout 0 do
    
            timeNow = state.time
            
            info(timeNow, state.time, timeNow)
    
            state.onTimeout 5 do
            
                info(timeNow, state.time, timeNow + 5)
            
                assert_equal timeNow + 5, state.time
                
            end
            
            state.onTimeout 10 do
            
                info(timeNow, state.time, timeNow + 10)
            
                assert_equal timeNow + 10, state.time
            
            end
            
            state.onTimeout 15 do
            
                info(timeNow, state.time, timeNow + 15)
            
                assert_equal timeNow + 15, state.time
            
            end
            
            state.onTimeout 20 do
            
                info(timeNow, state.time, timeNow + 20)
            
                assert_equal timeNow + 20, state.time
            
                q.push nil
            
            end
            
        end
        
        Timeout::timeout 1 do
            q.pop
        end
        
    end
    
    def teardown
        @state.stop
        Thread.abort_on_exception = false
    end


end
