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
    
    def teardown
        @state.stop
        Thread.abort_on_exception = false
    end


end
