require 'json'

module LDL::Semtech

    class TXPacket
    
=begin
         Name |  Type  | Function
        :----:|:------:|--------------------------------------------------------------
         imme | bool   | Send packet immediately (will ignore tmst & time)
         tmst | number | Send packet on a certain timestamp value (will ignore time)
         tmms | number | Send packet at a certain GPS time (GPS synchronization required)
         freq | number | TX central frequency in MHz (unsigned float, Hz precision)
         rfch | number | Concentrator "RF chain" used for TX (unsigned integer)
         powe | number | TX output power in dBm (unsigned integer, dBm precision)
         modu | string | Modulation identifier "LORA" or "FSK"
         datr | string | LoRa datarate identifier (eg. SF12BW500)
         datr | number | FSK datarate (unsigned, in bits per second)
         codr | string | LoRa ECC coding rate identifier
         fdev | number | FSK frequency deviation (unsigned integer, in Hz) 
         ipol | bool   | Lora modulation polarization inversion
         prea | number | RF preamble size (unsigned integer)
         size | number | RF packet payload size in bytes (unsigned integer)
         data | string | Base64 encoded RF packet payload, padding optional
         ncrc | bool | If true, disable the CRC of the physical layer (optional)
=end
    
        def self.from_json
            begin
                self.new(
                    imme: msg["imme"],
                    tmst: msg["tmst"],
                    tmms: msg["tmms"],
                    freq: msg["freq"],
                    rfch: msg["rfch"],
                    powe: msg["powe"],
                    modu: msg["modu"],
                    datr: msg["datr"],
                    codr: msg["codr"],
                    fdev: msg["fdev"],
                    ipol: msg["ipol"],
                    prea: msg["prea"],
                    size: msg["size"],
                    data: msg["data"],
                    ncrc: msg["ncrc"]
                )        
            rescue
                raise ArgumentError
            end            
        end
        
        attr_reader :imme
        attr_reader :tmst
        attr_reader :tmms
        attr_reader :freq
        attr_reader :rfch
        attr_reader :powe
        attr_reader :modu
        attr_reader :datr
        attr_reader :codr
        attr_reader :fdev
        attr_reader :ipol
        attr_reader :prea
        
        def size
            @data.size
        end
        
        attr_reader :data
        attr_reader :ncrc
    
        def initialize(**param)
            init = Proc.new do |iv_name, klass, default|                
                if param[iv_name]
                    raise TypeError unless klass == nil or param[iv_name].kind_of? klass                                        
                    if block_given?
                        value = yield(param[iv_name])
                    else
                        value = param[iv_name]                    
                    end
                    instance_variable_set("@#{iv_name}", value)
                else
                    instance_variable_set("@#{iv_name}", default)        
                end                
            end            
            
            init.call(:imme, nil, true) do |value|
                ( value ? true : false )
            end 
            
            init.call(:tmst, Integer, 0) 
            init.call(:tmms, Integer, 0) 
            init.call(:freq, Numeric, 0) 
            init.call(:rfch, Integer, 0) 
            init.call(:powe, Integer, 0) 
            
            init.call(:modu, String, "LORA") do |value|
                raise RangeError unless ["LORA", "FSK"].include? value
                value            
            end
                        
            if param[:datr]
                if modu == "LORA"
                    if not(match = param[:datr].match(/^SF(?<sf>[0-9])BW(?<bw>[0-9]+)$/))
                        raise ArgumentError
                    end
                    @datr = param[:datr]
                    @sf = match[:sf]
                    @bw = match[:bw]
                else
                    raise ArugmentError unless not(param[:datr].kind_of? Numeric)
                end
            else
                if modu == "LORA"            
                    @datr = "SF7BW125"
                    @bw = 125000
                    @sf = 7
                else
                    @datar = 50000                
                end
            end
            
            init.call(:codr, String, "4/5") do |value|
                raise ArgumentError unless value[/^[0-9]+\/[0-9]+$/]
                value
            end
            
            init.call(:fdev, Integer, 0) 
            init.call(:ipol, Integer, 0) 
            init.call(:prea, Integer, 0) 
            init.call(:data, String, "") 
            
            if param[:size]            
                raise TypeError unless param[:size].kind_of? Numeric
                raise ArgumentError.new("explicit size does not match size of data") unless param[:size] == data.size
            end
            
            init.call(:ncrc, nil, true) do |value|
                ( value ? true : false )
            end
            
        end
        
        def to_json
            {
                :imme => imme,
                :tmst => tmst,
                :tmms => tmms,
                :freq => freq,
                :rfch => rfch,
                :powe => powe,
                :modu => modu,
                :datr => datr,
                :codr => codr,                
                :fdev => fdev,
                :ipol => ipol,
                :prea => prea,
                :size => size,
                :data => Base64.encode64(@data),
                :ncrc => ncrc
            }.to_json
        end
    
    end
    
end
