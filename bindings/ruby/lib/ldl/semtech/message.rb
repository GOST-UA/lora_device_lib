module LDL::Semtech

    class Message
    
        VERSION = 2
    
        @subclasses = []
        @type = nil
        
        def self.type
            @type
        end
        
        def self.inherited(subclass)
            @subclasses << subclass
        end
    
        def self.decode(msg)
        
            iter = msg.unpack("Ca2Ca").each
            
            version = iter.next
            nonce = iter.next
            type = iter.next
            payload = iter.next
            
            if version != 2
                raise
            end
            
            if not (message_type = @subclasses.detect{|m|m.type == type})
                raise
            end
            
            message_type.decode(msg)            
            
        end
        
        attr_reader :token
        
        def encode
            [VERSION, token, self.class.type].pack("Ca2C")
        end
    
    end

end
