require 'minitest/autorun'
require 'ldl'
require 'timeout'

class TestTimeoutQueue < Minitest::Test

    include LDL

    def setup
        @queue = TimeoutQueue.new
    end

    def test_pop_with_timeout

        assert_raises ThreadError do

            Timeout::timeout 0.5 do

                @queue.pop 0.25

            end

        end

    end


end
