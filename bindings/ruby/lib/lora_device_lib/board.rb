module LoraDeviceLib

    class BoardError < StandardError        
    end

    class Board
        
        attr_reader :type
    
        def initialize
            raise "abstract class"
        end
        
    end
    
    class Frame
    
        def self.decode(input)
        end
        
        def initialize(**attr)
        end
        
        def encode
        end
    
    end
    
    class JoinReq < Frame
    end
    
    class JoinAccept < Frame
    end
    
    class DataFrame < Frame
    end
    
    class UnconfirmedUp < DataFrame
    end
    
    class ConfirmedUp < DataFrame
    end
    
    class UnconfirmedDown < DataFrame
    end
    
    class ConfirmedDown < DataFrame
    end
    
end
