module LoraDeviceLib

    class SPIBuffer
    
        MAX_SIZE = 0xff
    
        attr_accessor :read_base
        attr_accessor :write_base
        attr_accessor :addr_ptr
        attr_accessor :rx_ptr
    
        def initialize(ptr, read_base, write_base)
            @buffer = Array.new(0, MAX_SIZE)
            @read_base = 0
            @write_base = 0
            @addr_ptr = 0
            @rx_ptr = 0
        end
        
        
        
        def write(data)
            
        
            addr = addr_ptr % MAX_SIZE
            @buffer[addr] = data
            addr += 1
        end
        
        def spi_write
            
        end
        
        def spi_read
        end
        
        def harness_write            
        end
        
        def harness_read            
        end

        private
        
            def addr(ptr)
                ptr % MAX_SIZE
            end        
            def incr(ptr)
                ptr + 1
            end
        
    end

    class Board_sx1272 < Board
    
        # max addr
        MAX_ADDR = 0xff
    
        REG_MAP = {
        
            :REG_LR_FIFO => {
                :addr => 0x00,
                :por => 0x00                
            },
            :REG_LR_OPMODE => {
                :addr => 0x01,
                :por => 0x01
            },
            :REG_LR_FRFMSB => {                               
                :addr => 0x06,
                :por => 0xe4
            },
            :REG_LR_FRFMID => {                               
                :addr => 0x07,
                :por => 0xc0                
            },
            :REG_LR_FRFLSB => {                               
                :addr => 0x08,
                :por => 0x00
            },
            :REG_LR_PACONFIG => {                             
                :addr => 0x09,
                :por => 0x0f
            },
            :REG_LR_PARAMP => {                               
                :addr => 0x0A,
                :por => 0x19
            },
            :REG_LR_OCP => {                                  
                :addr => 0x0B,
                :por => 0x2b
            },
            :REG_LR_LNA => {                                  
                :addr => 0x0C,
                :por => 0x20
            },
            :REG_LR_FIFOADDRPTR => {                          
                :addr => 0x0D,
                :por => 0x08
            },
            :REG_LR_FIFOTXBASEADDR => {                       
                :addr => 0x0E,
                :por => 0x02
            },
            :REG_LR_FIFORXBASEADDR => {                       
                :addr => 0x0F,
                :por => 0x0a
            },
            :REG_LR_FIFORXCURRENTADDR => {                    
                :addr => 0x10,
                :por => 0xff
            },
            :REG_LR_IRQFLAGSMASK => {                         
                :addr => 0x11,
                :por => 0x00
            },
            :REG_LR_IRQFLAGS => {                             
                :addr => 0x12,
                :por => 0x15
            },
            :REG_LR_RXNBBYTES => {                            
                :addr => 0x13,
                :por => 0x0b
            },
            :REG_LR_RXHEADERCNTVALUEMSB => {                  
                :addr => 0x14,
                :por => 0x28
            },
            :REG_LR_RXHEADERCNTVALUELSB => {                  
                :addr => 0x15,
                :por => 0x0c
            },
            :REG_LR_RXPACKETCNTVALUEMSB => {                  
                :addr => 0x16,
                :por => 0x12
            },
            :REG_LR_RXPACKETCNTVALUELSB => {                  
                :addr => 0x17,
                :por => 0x47
            },
            :REG_LR_MODEMSTAT => {                            
                :addr => 0x18,
                :por => 0x32
            },
            :REG_LR_PKTSNRVALUE => {                          
                :addr => 0x19,
                :por => 0x3e
            },
            :REG_LR_PKTRSSIVALUE => {                         
                :addr => 0x1A,
                :por => 0x00
            },
            :REG_LR_RSSIVALUE => {                            
                :addr => 0x1B,
                :por => 0x00
            },
            :REG_LR_HOPCHANNEL => {                           
                :addr => 0x1C,
                :por => 0x00
            },
            :REG_LR_MODEMCONFIG1 => {                         
                :addr => 0x1D,
                :por => 0x00
            },
            :REG_LR_MODEMCONFIG2 => {                         
                :addr => 0x1E,
                :por => 0x00
            },
            :REG_LR_SYMBTIMEOUTLSB => {                       
                :addr => 0x1F,
                :por => 0x40
            },
            :REG_LR_PREAMBLEMSB => {                          
                :addr => 0x20,
                :por => 0x00
            },
            :REG_LR_PREAMBLELSB => {                          
                :addr => 0x21,
                :por => 0x00
            },
            :REG_LR_PAYLOADLENGTH => {                        
                :addr => 0x22,
                :por => 0x00
            },
            :REG_LR_PAYLOADMAXLENGTH => {                     
                :addr => 0x23,
                :por => 0x00
            },
            :REG_LR_HOPPERIOD => {                            
                :addr => 0x24,
                :por => 0x05
            },
            :REG_LR_FIFORXBYTEADDR => {                       
                :addr => 0x25,
                :por => 0x00
            },
            :REG_LR_FEIMSB => {                               
                :addr => 0x28,
                :por => 0x55
            },
            :REG_LR_FEIMID => {                               
                :addr => 0x29,
                :por => 0x55
            },
            :REG_LR_FEILSB => {                               
                :addr => 0x2A,
                :por => 0x55
            },
            :REG_LR_RSSIWIDEBAND => {                          
                :addr => 0x2C,
                :por => 0x55
            },
            :REG_LR_DETECTOPTIMIZE => {                       
                :addr => 0x31,
                :por => 0x40
            },
            :REG_LR_INVERTIQ => {                             
                :addr => 0x33,
                :por => 0x00
            },
            :REG_LR_DETECTIONTHRESHOLD => {                   
                :addr => 0x37,
                :por => 0x00
            },
            :REG_LR_SYNCWORD => {                             
                :addr => 0x39,
                :por => 0xf5
            },
            :REG_LR_INVERTIQ2 => {                            
                :addr => 0x3B,
                :por => 0x82
            },

            :REG_LR_DIOMAPPING1 => {                          
                :addr => 0x40,
                :por => 0x00
            },
            :REG_LR_DIOMAPPING2 => {                          
                :addr => 0x41,
                :por => 0x00
            },
            :REG_LR_VERSION => {                              
                :addr => 0x42,
                :por => 0x22
            },
            :REG_LR_AGCREF => {                               
                :addr => 0x43,
                :por => 0x13
            },
            :REG_LR_AGCTHRESH1 => {                           
                :addr => 0x44,
                :por => 0x0e
            },
            :REG_LR_AGCTHRESH2 => {                           
                :addr => 0x45,
                :por => 0x5b
            },
            :REG_LR_AGCTHRESH3 => {                           
                :addr => 0x46,
                :por => 0xdb
            },
            :REG_LR_PLLHOP => {                               
                :addr => 0x4B,
                :por => 0x2e
            },
            :REG_LR_TCXO => {                                 
                :addr => 0x58,
                :por => 0x09
            },
            :REG_LR_PADAC => {                                
                :addr => 0x5A,
                :por => 0x84
            },
            :REG_LR_PLL => {                                  
                :addr => 0x5C,
                :por => 0xd0
            },
            :REG_LR_PLLLOWPN => {                             
                :addr => 0x5E,
                :por => 0xd0
            },
            :REG_LR_FORMERTEMP => {                           
                :addr => 0x6C,  # stored temperature
                :por => 0x00   # stored temperature
            }            
        }
        
        def initialize
            @type = :LORA_RADIO_SX1272
            @reset = false
            @select = false
            @fifo = LoraFifo.new
            reset()            
        end
        
        # lookup a register attributes
        #
        # @param key [Symbol, Integer]
        # @return attr [Hash,nil]
        #
        # @options attr [Symbol] :name register name
        # @options attr [Integer] :addr register address
        # @options attr [Integer] :por power on reset value
        def self.lookup_register_attr(key)
            if key.kind_of? Symbol
                result = REG_MAP[key]
                if result
                    result.merge({:name => key})
                end
            else
                REG_MAP.each do |k, v|
                    if v[:addr] == key
                        return v.merge({:name => key})
                    end
                end
                nil
            end
        end
        
        def lookup_register_attr(key)
            self.class.lookup_register_attr(key)
        end
        
        # reset the internal register to the "power on reset" setting
        def reset
            @register = Array.new(MAX_ADDR, 0)
            REG_MAP.values.each do |r|
                @register[r[:addr]] = r[:por]
            end
            @buffer = []
            @ptr = 0
            @addr = 0
            @mode = nil
            self
        end
    
        # set the select line
        def select_on
            if @reset
                raise BoardError.new "select during reset"
            else
                if @select
                    raise BoardError.new "double select on"                
                else
                    puts __method__
                    @select = true
                    @buffer = []
                end            
            end
        end
        
        # release the select line
        def select_off       
            if @select
                @select = false
                puts __method__               
                apply_buffer(@buffer)
                @mode = nil                
            else
                raise BoardError.new "double select off"                                
            end            
        end
                
        # set the reset line
        def reset_on
            if @reset
                raise BoardError.new "double reset on"                
            else
                puts __method__
                reset()
                @reset = true
            end
        end
        
        # release the reset line
        def reset_off
            if @reset
                puts __method__
                @reset = false
            else
                raise BoardError.new "double reset off"                                
            end
        end
        
        # write a byte
        def write(data)
        
            if @reset
                raise BoardError.new "write during reset"
            else
                if @select            
                    
                    case @mode
                    when nil
                        @addr = data & 0x7f                    
                        @ptr = @addr
                        @mode = ( (data & 0x80) == 0 ? :read : :write )                    
                    else                                
                        @buffer << data                                        
                        addr_and_increment()
                        self
                    end                
                    puts "wrote 0x#{ "%02X" % data }"                
                else            
                    raise BoardError.new "write without select"                
                end
            end
            
        end
                
        # read a byte
        def read
        
            if @reset
                raise BoardError.new "read during reset"
            else
                if @select                          
                    read_register(add_and_increment)                                  
                else            
                    raise BoardError.new "read without select"            
                end
            end
        
        end
        
        # get the current address and increment the pointer
        def addr_and_increment
            result = @ptr % MAX_ADDR
            @ptr += 1
            result
        end
        
        # apply the contents of @buffer to @register 
        #
        def apply_buffer(buffer)
                
            buffer.each.with_index do |b, index|
        
                addr = (@addr + index) % @register.size            
                write_register(addr, b)
        
            end
        
        end
            
            def write_register(addr, data)
                    
                if reg = lookup_reg(addr)
                    
                    @register[addr] = b
                
                    case reg[:name]
                    when :REG_LR_FIFO          
                    
                        fifo.write(b)
                                  
                    when :REG_LR_OPMODE
                    when :REG_LR_FRFMSB
                    when :REG_LR_FRFMID
                    when :REG_LR_FRFLSB
                    when :REG_LR_PACONFIG
                    when :REG_LR_PARAMP
                    when :REG_LR_OCP
                    when :REG_LR_LNA
                    when :REG_LR_FIFOADDRPTR
                    when :REG_LR_FIFOTXBASEADDR
                    when :REG_LR_FIFORXBASEADDR
                    when :REG_LR_FIFORXCURRENTADDR
                    when :REG_LR_IRQFLAGSMASK
                    when :REG_LR_IRQFLAGS
                    when :REG_LR_RXNBBYTES
                    when :REG_LR_RXHEADERCNTVALUEMSB
                    when :REG_LR_RXHEADERCNTVALUELSB
                    when :REG_LR_RXPACKETCNTVALUEMSB
                    when :REG_LR_RXPACKETCNTVALUELSB
                    when :REG_LR_MODEMSTAT
                    when :REG_LR_PKTSNRVALUE
                    when :REG_LR_PKTRSSIVALUE
                    when :REG_LR_RSSIVALUE
                    when :REG_LR_HOPCHANNEL
                    when :REG_LR_MODEMCONFIG1
                    when :REG_LR_MODEMCONFIG2
                    when :REG_LR_SYMBTIMEOUTLSB
                    when :REG_LR_PREAMBLEMSB
                    when :REG_LR_PREAMBLELSB
                    when :REG_LR_PAYLOADLENGTH
                    when :REG_LR_PAYLOADMAXLENGTH
                    when :REG_LR_HOPPERIOD
                    when :REG_LR_FIFORXBYTEADDR
                    when :REG_LR_FEIMSB
                    when :REG_LR_FEIMID
                    when :REG_LR_FEILSB
                    when :REG_LR_RSSIWIDEBAND
                    when :REG_LR_DETECTOPTIMIZE
                    when :REG_LR_INVERTIQ
                    when :REG_LR_DETECTIONTHRESHOLD
                    when :REG_LR_SYNCWORD
                    when :REG_LR_INVERTIQ2
                    when :REG_LR_DIOMAPPING1
                    when :REG_LR_DIOMAPPING2
                    when :REG_LR_VERSION
                    when :REG_LR_AGCREF
                    when :REG_LR_AGCTHRESH1
                    when :REG_LR_AGCTHRESH2
                    when :REG_LR_AGCTHRESH3
                    when :REG_LR_PLLHOP
                    when :REG_LR_TCXO
                    when :REG_LR_PADAC
                    when :REG_LR_PLL
                    when :REG_LR_PLLLOWPN
                    when :REG_LR_FORMERTEMP
                    end                
                end
            end
            
            def read_register(addr)
                
                if reg = lookup_reg(addr)
                    
                    @register[addr] = b
                
                    case reg[:name]
                    when :REG_LR_FIFO                                  
                    when :REG_LR_OPMODE
                    when :REG_LR_FRFMSB
                    when :REG_LR_FRFMID
                    when :REG_LR_FRFLSB
                    when :REG_LR_PACONFIG
                    when :REG_LR_PARAMP
                    when :REG_LR_OCP
                    when :REG_LR_LNA
                    when :REG_LR_FIFOADDRPTR
                    when :REG_LR_FIFOTXBASEADDR
                    when :REG_LR_FIFORXBASEADDR
                    when :REG_LR_FIFORXCURRENTADDR
                    when :REG_LR_IRQFLAGSMASK
                    when :REG_LR_IRQFLAGS
                    when :REG_LR_RXNBBYTES
                    when :REG_LR_RXHEADERCNTVALUEMSB
                    when :REG_LR_RXHEADERCNTVALUELSB
                    when :REG_LR_RXPACKETCNTVALUEMSB
                    when :REG_LR_RXPACKETCNTVALUELSB
                    when :REG_LR_MODEMSTAT
                    when :REG_LR_PKTSNRVALUE
                    when :REG_LR_PKTRSSIVALUE
                    when :REG_LR_RSSIVALUE
                    when :REG_LR_HOPCHANNEL
                    when :REG_LR_MODEMCONFIG1
                    when :REG_LR_MODEMCONFIG2
                    when :REG_LR_SYMBTIMEOUTLSB
                    when :REG_LR_PREAMBLEMSB
                    when :REG_LR_PREAMBLELSB
                    when :REG_LR_PAYLOADLENGTH
                    when :REG_LR_PAYLOADMAXLENGTH
                    when :REG_LR_HOPPERIOD
                    when :REG_LR_FIFORXBYTEADDR
                    when :REG_LR_FEIMSB
                    when :REG_LR_FEIMID
                    when :REG_LR_FEILSB
                    when :REG_LR_RSSIWIDEBAND
                    when :REG_LR_DETECTOPTIMIZE
                    when :REG_LR_INVERTIQ
                    when :REG_LR_DETECTIONTHRESHOLD
                    when :REG_LR_SYNCWORD
                    when :REG_LR_INVERTIQ2
                    when :REG_LR_DIOMAPPING1
                    when :REG_LR_DIOMAPPING2
                    when :REG_LR_VERSION
                    when :REG_LR_AGCREF
                    when :REG_LR_AGCTHRESH1
                    when :REG_LR_AGCTHRESH2
                    when :REG_LR_AGCTHRESH3
                    when :REG_LR_PLLHOP
                    when :REG_LR_TCXO
                    when :REG_LR_PADAC
                    when :REG_LR_PLL
                    when :REG_LR_PLLLOWPN
                    when :REG_LR_FORMERTEMP
                    end                
                end
            end
        
        end
        
    end


end
