module LDL::Semtech

    class PushAck < Message
    
        @type = 1
    
        def encode
            [super, @eui.bytes, self.to_json].pack("aaa")
        end
    
    end

end
