// pfm2sid: PreenFM2 meets SID
//
// Copyright (C) 2023-2024 Patrick Dowling (pld@gurkenkiste.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
#include "dac_4922.h"

#include "drivers/pfm2sid_gpio.h"
#include "stm32x/stm32x_gpio_utils.h"

namespace pfm2sid {

void Dac::Init()
{
  stm32x::Init<GPIO::DAC_R_CS, GPIO::DAC_L_CS, GPIO::DAC_LDAC>();
  stm32x::Set<GPIO::DAC_R_CS, GPIO::DAC_L_CS, GPIO::DAC_LDAC>();

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
  stm32x::Init<GPIO::SPI1_SCK, GPIO::SPI1_MOSI>();

  SPI_InitTypeDef spi_init;
  spi_init.SPI_Direction = SPI_Direction_1Line_Tx;
  spi_init.SPI_Mode = SPI_Mode_Master;
  spi_init.SPI_DataSize = SPI_DataSize_16b;
  spi_init.SPI_CPOL = SPI_CPOL_Low;
  spi_init.SPI_CPHA = SPI_CPHA_1Edge;
  spi_init.SPI_NSS = SPI_NSS_Soft;
  spi_init.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;  // 84/8 => 10.5MHz
  spi_init.SPI_FirstBit = SPI_FirstBit_MSB;
  spi_init.SPI_CRCPolynomial = 7;

  SPI_Init(SPI1, &spi_init);
  SPI_Cmd(SPI1, ENABLE);
}

void Dac::BeginFrame(int32_t l, int32_t r)
{
  static constexpr uint16_t kChannelSelect[2] = {
      MCP4922::SELECT_A | MCP4922::GAIN_1X | MCP4922::ACTIVE,
      MCP4922::SELECT_B | MCP4922::GAIN_1X | MCP4922::ACTIVE,
  };
  static_assert(0x3000 == kChannelSelect[0]);
  static_assert(0xb000 == kChannelSelect[1]);

  // ssat:  -2^sat-1 ≤ x ≤ 2^sat-1 -1

  uint32_t ul = kZeroValue + __SSAT(l, kResolution);
  uint32_t ur = kZeroValue + __SSAT(r, kResolution);

  values_[0] = kChannelSelect[0] | (ur >> 6);
  values_[1] = kChannelSelect[0] | (ul >> 6);
  values_[2] = kChannelSelect[1] | (ur & 0x3f);
  values_[3] = kChannelSelect[1] | (ul & 0x3f);
  state_ = 0;

  GPIO::DAC_LDAC::Set();
  Transfer<DAC_R>(values_[0]);
}
}  // namespace pfm2sid
