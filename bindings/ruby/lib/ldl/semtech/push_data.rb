module LDL::Semtech

    class PushData < Message
    
        @type = 0
    
        def to_message
            [2, @nonce, 0x00, @eui.bytes, self.to_json].pack("Ca2Ca8a")
        end
    
        def encode
            [super, @eui.bytes, self.to_json].pack("aaa")
        end
    
    end

end
