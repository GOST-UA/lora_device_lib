module LoraDeviceLib

    class BoardError < StandardError        
    end

    class Board
        
        attr_reader :type
        
    end

end
