module LDL

    class Key
    
        attr_reader :value
    
        def initialize(value)
            if !value.kind_of? String or value.size != 16
                raise TypeError
            end
            @value = value
        end
    
    end

end
