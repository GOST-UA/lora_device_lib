module LDL::Semtech

    class TXAck < Message
    
        @type = 5

        def self.decode(msg)
            
            iter = msg.unpack("CS>Ca8a*").each
            
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
            
            begin            
                
                root = JSON.parse(iter.next)
                
                if not root.kind_of? Hash
                    raise ArgumentError.new "expecting root JSON to be an object"
                end
                
                if root["txpk_ack"].nil?
                    raise ArgumentError.new "root must contain the key 'txpk_ack'"
                end
                
                txpk_ack = root["txpk_ack"].map{|p|TXPacketAck.from_h(root["txpk_ack"])}
                
            rescue           
                ArgumentError.new "payload is not valid"            
            end
            
            self.new(
                version: version,
                token: token,
                eui: eui,
                txpk_ack: txpk_ack                
            )
        end

        attr_reader :eui
        attr_reader :rxpk_ack

        def initialize(**param)
        
            super(**params)
                               
            if params[:txpk_ack]
                @rxpk_ack = params[:txpk_ack]
            else
                @rxpk_ack = {:txpk_ack => {}}     
            end
            
            if params[:eui]
                raise TypeError unless params[:eui].kind_of? LDL::EUI64
                @eui = params[:eui]                
            else
                @eui = LDL::EUI64.new("00-00-00-00-00-00-00-00")
            end
        
        end

        def encode
            [super, eui, rxpk_ack.to_json].pack("a*a*a*")
        end

    end

end
