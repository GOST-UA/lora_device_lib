module LDL::Semtech

    class PullAck < Message
    
        @type = 4
    
        def encode
            [super,].pack("")
        end
    
    end

end
