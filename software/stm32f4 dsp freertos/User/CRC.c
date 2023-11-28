#include "CRC.h"

#include <stdio.h>
#include <stdint.h>

//// CRC校验函数
//uint16_t crc16(uint8_t *data, int length)
//{
//    uint16_t crc = 0xFFFF;
//    
//    for (int i = 0; i < length; i++)
//    {
//        crc ^= data[i];
//        
//        for (int j = 0; j < 8; j++)
//        {
//            if (crc & 1)
//            {
//                crc >>= 1;
//                crc ^= 0xA001;
//            }
//            else
//            {
//                crc >>= 1;
//            }
//        }
//    }
//    
//    return crc;
//}

//// 封装CRC校验值到数据中
//void appendCRC(uint8_t *data, int length)
//{
//    uint16_t crc = crc16(data, length);
//    data[length] = crc & 0xFF; // 将低8位放入数据末尾
//    data[length + 1] = crc >> 8; // 将高8位放入数据末尾的下一个位置
//}


//// 验证CRC校验值是否正确
//int verifyCRC(uint8_t *data, int length)
//{
//    uint16_t crc = crc16(data, length - 2); // 去除数据末尾的CRC校验值
//    
//    // 获取接收到的CRC校验值
//    uint16_t receivedCRC = (data[length - 1] << 8) | data[length - 2];
//    
//    // 比较计算得到的CRC校验值与接收到的CRC校验值
//    if (crc == receivedCRC)
//    {
//        return 1; // 校验通过
//    }
//    else
//    {
//        return 0; // 校验失败
//    }
//}
