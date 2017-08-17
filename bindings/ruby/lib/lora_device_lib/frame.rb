module LoraDeviceLib

    class Frame
    end    
    class JoinReq < Frame
        
        attr_reader :appKey
    
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
                if !@appEUI.kind_of? EUI64
                    raise TypeError
                end
            else
                @appEUI = EUI64.new("00-00-00-00-00-00-00-00")
            end
            
            if @devEUI
                if !@devEUI.kind_of? EUI64
                    raise TypeError
                end
            else
                @devEUI = EUI64.new("00-00-00-00-00-00-00-00")
            end
            
            if @devNonce
                if !@devNonce.kind_of? Integer
                    raise TypeError
                end
            else
                @devNonce = 0
            end
            
        end
    end
    class JoinAccept < Frame
    
        attr_reader :appKey
    
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
                if !@netID.kind_of? Integer
                    raise TypeError
                end
            else
                @netID = 0
            end
            
            if @devAddr
                if !@devAddr.kind_of? Integer
                    raise TypeError
                end
            else
                @devAddr = 0
            end
            
            if @rx1DataRateOffset
                if !Range.new(0,255).include? @rx1DataRateOffset
                    raise TypeError
                end
            else
                @rx1DataRateOffset = 0
            end
            
            if @rx2DataRate
                if !Range.new(0,255).include? @rx2DataRate
                    raise TypeError
                end
            else
                @rx2DataRate = 0
            end
            
            if @rxDelay
                if !Range.new(0,255).include? @rxDelay
                    raise TypeError
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
                if !Range.new(0,0xffffff).include? @appNonce
                    raise TypeError
                end
            else
                @appNonce = 0
            end
            
        end
    end
    class DataFrame < Frame
    
        attr_reader :appSKey
        attr_reader :nwkSKey
        
        def initialize(**param)
            
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
                if !@nwkSKey.kind_of? Key
                    raise TypeError
                end
            else
                @nwkSKey = Key.new("\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00")
            end
            
            if @appSKey
                if !@appSKey.kind_of? Key
                    raise TypeError
                end
            else
                @appSKey = Key.new("\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00")
            end
            
            if @counter
                if !Range.new(0, 65535).include? @counter  
                    raise TypeError.new ":counter must be an integer in the range 0..65535"
                end
            else
                @counter = 0
            end
            
            if @devAddr
                if !@devAddr.kind_of? Integer
                    raise TypeError.new ":devAddr must be an integer"
                end
            else
            
                @devAddr = 0
            end
            
            if @opts
                if !@opts.kind_of? String or @opts.size > 16
                    raise TypeError
                end
            else
                @opts = ""
            end
            
            if @data
                if !@data.kind_of? String
                    raise TypeError
                end
            else
                @opts = ""
            end
            
            if @port
                if !@port.kind_of? Integer or @port > 255
                    raise TypeError
                end
            else
                @port = 0
            end
            
            if @data
                if !@data.kind_of? String or @data.size > 255
                    raise TypeError
                end
            else
                @data = ""
            end
        
        end    
    end
    
end

require 'lora_device_lib/ext_frame'
