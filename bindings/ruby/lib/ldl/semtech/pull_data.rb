module LDL::Semtech

    class PullData < Message
    
        @type = 2
        
        def self.decode(msg)
            
            iter = msg.unpack("CS>Ca8").each
            
            version = iter.next
            token = iter.next
            type = iter.next
            eui = iter.next
            
            if version != Message::VERSION
                raise ArgumentError.new "unknown protocol version"
            end
            
            if type != self.type
                raise ArgumentError.new "expecting message type #{self.type}"
            end
            
            self.new(
                version: version,
                token: token,
                eui: LDL::EUI64.new(eui),
            )
        end
    
        attr_reader :eui
    
        def initialize(**params)
            
            super(**params)
            
            if params.has_key? :eui
                raise TypeError unless params[:eui].kind_of? LDL::EUI64
                @eui = params[:eui]                
            else
                @eui = LDL::EUI64.new("00-00-00-00-00-00-00-00")
            end
            
        end
    
        def encode
            [super, eui.bytes].pack("a*a*")
        end
    
    end

end
