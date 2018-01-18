require 'minitest/autorun'
require 'ldl'

class TestTimeSource < Minitest::Test

    include LDL

    def setup
        @source = TimeSource.new
        @source.start
    end

    def test_onTimeout

=begin    
        q = Queue.new
    
        @source.onTimeout 100000 do
        
            q.push nil
        
        end
        
        q.pop
=end        
        
    end
    
    def teardown
        @source.stop
    end


end
