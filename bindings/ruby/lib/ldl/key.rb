module LDL

    class Key
    
        attr_reader :value
    
        def initialize(value)
            raise TypeError unless value.kind_of? String
            puts value.size
            raise ArgumentEror unless value.size == 16
            @value = value
        end
    
    end

end
