module LDL

    class Frame
    
        def initialize(**param)
            if self.class == Frame
                raise "#{self.class} is an abstract class"
            end
        end
    
    end    
    
    class JoinReq < Frame
        
        attr_reader :appKey
        
        attr_reader :original
        
        attr_reader :appEUI
        attr_reader :devEUI
        attr_reader :devNonce
    
        def initialize(**param)
            
            @original = param[:original]
            @appKey = param[:appKey]
            @appEUI = param[:appEUI]
            @devEUI = param[:devEUI]
            @devNonce = param[:devNonce]
                        
            if @appKey
                if !@appKey.kind_of? Key
                    raise TypeError
                end
            else
                @appKey = Key.new("\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00")
            end
            
            if @appEUI
                if not @appEUI.kind_of? EUI64
                    raise TypeError
                end
            else
                @appEUI = EUI64.new("00-00-00-00-00-00-00-00")
            end
            
            if @devEUI
                if not @devEUI.kind_of? EUI64
                    raise TypeError
                end                
            else
                @devEUI = EUI64.new("00-00-00-00-00-00-00-00")
            end
            
            if @devNonce
                if !@devNonce.kind_of? Integer
                    raise TypeError
                end
                if not (0..0xffff).include? @devNonce
                    raise ArgumentError
                end
            else
                @devNonce = 0
            end
            
        end
    end
    class JoinAccept < Frame
    
        attr_reader :appKey
        
        attr_reader :original
        
        attr_reader :netID
        attr_reader :devAddr
        attr_reader :rx1DataRateOffset
        attr_reader :rx2DataRate
        attr_reader :rxDelay
        attr_reader :cfList
        attr_reader :appNonce
    
        def initialize(**param)
            
            @original = param[:original]
            @appKey = param[:appKey]
            @netID = param[:netID]
            @devAddr = param[:devAddr]
            @rx1DataRateOffset = param[:rx1DataRateOffset]
            @rx2DataRate = param[:rx2DataRate]
            @rxDelay = param[:rxDelay]
            @cfList = param[:cfList]
            @appNonce = param[:appNonce]
            
            if @appKey
                if !@appKey.kind_of? Key
                    raise TypeError
                end
            else
                @appKey = Key.new("\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00")
            end
            
            if @netID
                if not @netID.kind_of? Integer
                    raise TypeError
                end
                if not (0..0xffffff).include? @netID
                    raise ArgumentError
                end
            else
                @netID = 0
            end
            
            if @devAddr
                if !@devAddr.kind_of? Integer
                    raise TypeError
                end
                if not (0..0xffffffff).include? @devAddr
                    raise ArgumentError
                end
            else
                @devAddr = 0
            end
            
            if @rx1DataRateOffset
                if not @rx1DataRateOffset.kind_of? Integer
                    raise TypeError
                end            
                if not (0..0xff).include? @rx1DataRateOffset
                    raise ArgumentError
                end
            else
                @rx1DataRateOffset = 0
            end
            
            if @rx2DataRate
                if not @rx2DataRate.kind_of? Integer
                    raise TypeError
                end
                if not (0..255).include? @rx2DataRate
                    raise ArgumentError
                end
            else
                @rx2DataRate = 0
            end
            
            if @rxDelay
                if not @rxDelay.kind_of? Integer
                    raise TypeError
                end
                if not (0..255).include? @rxDelay
                    raise ArgumentError
                end
            else
                @rxDelay = 0
            end
            
            if @cfList
                # todo
            else
                @cfList = ""
            end
            
            if @appNonce
                if not @appNonce.kind_of? Integer
                    raise TypeError
                end
                if not (0..0xffffff).include? @appNonce
                    raise ArgumentError
                end
            else
                @appNonce = 0
            end
            
        end
    end
    class DataFrame < Frame
    
        attr_reader :appSKey
        attr_reader :nwkSKey
        
        attr_reader :original
        
        attr_reader :ack
        attr_reader :adr
        attr_reader :adrAckReq
        attr_reader :pending
        attr_reader :opts
        attr_reader :data
        attr_reader :port
        
        def initialize(**param)
            
            if self.class == DataFrame
                raise "#{self.class} is an abstract class"
            end
            
            @original = param[:original]
            
            @nwkSKey = param[:nwkSKey]
            @appSKey = param[:appSKey]
            @counter = param[:counter]
            @devAddr = param[:devAddr]
            
            @ack = ( param[:ack] ? true : false )
            @adr = ( param[:adr] ? true : false )
            @adrAckReq = ( param[:adrAckReq] ? true : false )
            @pending = ( param[:pending] ? true : false )
            
            @opts = param[:opts]
            @data = param[:data]
            @port = param[:port]
            
            if @nwkSKey
                if not @nwkSKey.kind_of? Key
                    raise TypeError
                end
            else
                @nwkSKey = Key.new("\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00")
            end
            
            if @appSKey
                if not @appSKey.kind_of? Key
                    raise TypeError
                end
            else
                @appSKey = Key.new("\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00")
            end
            
            if @counter
                if not @counter.kind_of? Integer
                    raise TypeError
                end
                if not (0..0xffff).include? @counter  
                    raise ArgumentError.new ":counter must be an integer in the range 0..65535"
                end
            else
                @counter = 0
            end
            
            if @devAddr
                if not @devAddr.kind_of? Integer
                    raise TypeError.new ":devAddr must be an integer"
                end
                if not (0..0xffffffff).include? @devAddr
                    raise ArgumentError
                end
            else            
                @devAddr = 0
            end
            
            if @opts
                if not @opts.kind_of? String
                    raise TypeError
                end
                if not (0..16).include? @opts.size
                    raise ArgumentError
                end
            else
                @opts = ""
            end
            
            if @port
                if not @port.kind_of? Integer
                    raise TypeError
                end
                if not (0..0xff).include? @port
                    raise ArgumentError
                end            
            else
                @port = 0
            end
            
            if @data
                if not @data.kind_of? String or @data.size > 255
                    raise TypeError
                end
                if not (0..0xff).include? @data.size
                    raise ArgumentError
                end
            else
                @data = ""
            end
        
        end    
    end
    
end

require 'lora_device_lib/ext_frame'
