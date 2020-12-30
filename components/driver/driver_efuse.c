#include <stdint.h>
#include <string.h>

#include "driver_efuse.h"
#include "driver_plf.h"

#include "co_math.h"

/*
 * TYPEDEFS (类型定义)
 */
struct efuse_reg_t {    
	uint32_t ctrl;    
	uint32_t d0;    
	uint32_t d1;    
	uint32_t d2;    
	uint32_t len_reg;
};

void efuse_write(uint32_t data0,uint32_t data1,uint32_t data2)
{
    volatile struct efuse_reg_t * const efuse_reg = (volatile struct efuse_reg_t *)(EFUSE_BASE);
    
    //ool_write(0x40,0x40);//读写efuse需将aldo电压降低到2.5V  
    efuse_reg->len_reg = 0x2814; 
    efuse_reg->d0 = data0;
    efuse_reg->d1 = data1;
    efuse_reg->d2 = data2;   
	
	efuse_reg->ctrl = 0x05;            
	while(((efuse_reg->ctrl)&CO_BIT(2)));            
	while(!((efuse_reg->ctrl)&CO_BIT(0)));    
    //efuse_reg->d0 = data0;
    //efuse_reg->d1 = data1;
    //efuse_reg->d2 = data2;   
}

void efuse_read(uint32_t *data0,uint32_t *data1,uint32_t *data2)
{
    volatile struct efuse_reg_t * const efuse_reg = (volatile struct efuse_reg_t *)(EFUSE_BASE);
    
	efuse_reg->ctrl = 0x03;
	while(((efuse_reg->ctrl)&CO_BIT(1)));            
	while(!((efuse_reg->ctrl)&CO_BIT(0)));    
	efuse_reg->len_reg = 0x2814; 
	*data0 = efuse_reg->d0;    
	*data1 = efuse_reg->d1;    
	*data2 = efuse_reg->d2;
}

void efuse_get_chip_unique_id(struct chip_unique_id_t * id_buff)
{
    uint8_t sloe_id_temp[6],i = 0;
    uint32_t temp = 0,w_id = 0,l_id = 0,coordinate_id = 0;
    uint32_t data[3];

    efuse_read(&data[0],&data[1],&data[2]);
        
    w_id = (data[1]>>8) & 0xff;
    for(i = 0;i < 8;i++)
    {
        temp = temp<<1;
        temp |= (w_id>>i)&0x01;        
    }
    w_id = temp & 0xff;
    sloe_id_temp[5] = w_id;
    temp = 0;

    l_id = ((data[1]&0xff)<<16) | ((data[0]&0xffff0000)>>16);
    for(i = 0;i < 24;i++)
    {
        temp = temp<<1;
        temp |= (l_id>>i)&0x01;
    }
    l_id = temp & 0xffffff;
    sloe_id_temp[4] = (l_id>>16)&0xff;
    sloe_id_temp[3] = (l_id>>8)&0xff;
    sloe_id_temp[2] = l_id&0xff;
    temp = 0;

    coordinate_id = (data[1]>>16)&0xffff;
    for(i = 0;i < 16;i++)
    {
        temp = temp<<1;
        temp |= (coordinate_id>>i)&0x01;
    }
    coordinate_id = temp & 0xffff;
    sloe_id_temp[1] = (coordinate_id>>8)&0xff;
    sloe_id_temp[0] = coordinate_id&0xff;

    memcpy(id_buff->unique_id,sloe_id_temp,6);
}


