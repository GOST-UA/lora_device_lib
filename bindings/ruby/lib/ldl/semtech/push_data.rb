require 'json'

module LDL::Semtech

    class PushData < Message
    
        @type = 0
        
        def self.decode(msg)
            
            iter = msg.unpack("CS>Ca8a").each
            
            version = iter.next
            token = iter.next
            type = iter.next
            eui = iter.next
            rxpk = nil
            stat = nil
            
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
                
                if root["rxpk"].nil? and root["stat"].nil?
                    raise ArgumentError.new "root should contain key(s) 'rxpk' or 'stat'"
                end
                
                if root["rxpk"]
                    rxpk = root["rxpk"].map{|p|RXPacket.from_h(root["rxpk"])}
                end
                
                if root["stat"]
                    stat = Status.from_h(root["stat"])
                end
                
            rescue           
                ArgumentError.new "payload is not valid"            
            end
            
            self.new(
                version: version,
                token: token,
                eui: LDL::EUI64.new(eui),
                rxpk: rxpk,
                stat: stat                
            )
             
        end
        
        attr_reader :eui
        attr_reader :rxpk
        attr_reader :stat
        
        def initialize(**params)
            
            super(**params)
                               
            if params[:rxpk]
                @rxpk = params[:rxpk]
            else
                @rxpk = []     
            end
            
            if params[:stat]
                @stat = params[:stat]
            end
            
            if params[:eui]
                raise TypeError unless params[:eui].kind_of? LDL::EUI64
                @eui = params[:eui]                
            else
                @eui = LDL::EUI64.new("00-00-00-00-00-00-00-00")
            end
            
        end
        
        def encode
        
            obj = {
                :rxpk => rxpk
            }
            if stat
                obj[:stat] = stat
            end

            obj.to_json

            [super, eui.bytes, obj.to_json].pack("a*a*a*")

        end
    
    end

end
