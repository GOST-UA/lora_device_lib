require "test/unit"
require 'lora_device_lib'

class TestBoardSX1272 < Test::Unit::TestCase

    include LoraDeviceLib

    def setup
        @board = Board_sx1272.new
    end

    def test_double_select_on
        @board.select_on
        assert_raise BoardError do
            @board.select_on
        end
    end

    def test_double_select_off
        assert_raise BoardError do
            @board.select_off
        end
    end

    def test_double_reset_on
        @board.reset_on
        assert_raise BoardError do
            @board.reset_on
        end
    end

    def test_double_reset_off
        assert_raise BoardError do
            @board.reset_off
        end
    end
    
    def test_write_during_reset
        @board.select_on
        @board.reset_on
        assert_raise BoardError do
            @board.write(42)
        end
    end
    
    def test_read_during_reset
        @board.select_on
        @board.reset_on
        assert_raise BoardError do
            @board.read
        end
    end
    
    def test_select_during_reset
        @board.reset_on
        assert_raise BoardError do
            @board.select_on
        end
    end
    
    def test_write_without_select
        assert_raise BoardError do
            @board.write(42)
        end
    end
    
    def test_read_without_select
        assert_raise BoardError do
            @board.read
        end
    end
    
    def test_read_por_registers
        buffer = []
        @board.select_on
        @board.write(0x00)
        while buffer.size < Board_sx1272::MAX_ADDR do
            buffer << @board.read
        end
        
        expected = []
        while expected.size < Board_sx1272::MAX_ADDR do
            reg = @board.lookup_register_attr(expected.size)
            if reg
                expected << reg[:por]
            else
                expected << 0
            end
        end
        
        assert_equal(expected, buffer)
    end
    
    
end
