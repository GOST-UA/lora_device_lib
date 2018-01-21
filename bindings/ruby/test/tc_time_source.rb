require 'minitest/autorun'
require 'ldl'
require 'timeout'

Thread.abort_on_exception = true

class TestTimeSource < Minitest::Test

    include LDL

    def setup
        @source = TimeSource.new
        @source.start
    end

    def test_wait

        Timeout::timeout 1 do
            @source.wait 9999
        end

    end

    def test_onTimeout

        q = Queue.new
    
        @source.onTimeout 999 do
        
            # #time must be predictable
            assert_equal 999, @source.time

            q.push nil
        
        end

        Timeout::timeout 1 do
            q.pop
        end
        
    end
    
    def teardown
        @source.stop
    end


end
