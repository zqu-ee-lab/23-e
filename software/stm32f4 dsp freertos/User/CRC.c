#include "CRC.h"

#include <stdio.h>
#include <stdint.h>

//// CRCУ�麯��
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

//// ��װCRCУ��ֵ��������
//void appendCRC(uint8_t *data, int length)
//{
//    uint16_t crc = crc16(data, length);
//    data[length] = crc & 0xFF; // ����8λ��������ĩβ
//    data[length + 1] = crc >> 8; // ����8λ��������ĩβ����һ��λ��
//}


//// ��֤CRCУ��ֵ�Ƿ���ȷ
//int verifyCRC(uint8_t *data, int length)
//{
//    uint16_t crc = crc16(data, length - 2); // ȥ������ĩβ��CRCУ��ֵ
//    
//    // ��ȡ���յ���CRCУ��ֵ
//    uint16_t receivedCRC = (data[length - 1] << 8) | data[length - 2];
//    
//    // �Ƚϼ���õ���CRCУ��ֵ����յ���CRCУ��ֵ
//    if (crc == receivedCRC)
//    {
//        return 1; // У��ͨ��
//    }
//    else
//    {
//        return 0; // У��ʧ��
//    }
//}
