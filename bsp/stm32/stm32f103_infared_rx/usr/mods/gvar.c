#include <stdlib.h>
#include "sys_conf.h"
#include "addr_drv.h"
#include "reg_chk.h"

//configuration parameters
sys_reg_st  g_sys; 																	//global parameter declairation

//configuration register map declairation
const conf_reg_map_st conf_reg_map_inst[CONF_REG_MAP_NUM]=  
{	//id		mapped registers		                 min	    max				    default			 type    chk_prt
    {	0,		&g_sys.conf.power_en,                    0,		    1,                  1,               0,       power_opt},		
    {	1,		&g_sys.conf.hwt_thl,                     10,		100,                49,              0,       NULL},
    {	2,		&g_sys.conf.hwt_thh,                     10,		5000,               195,             0,       NULL},
    {	3,		&g_sys.conf.dac_ref,                     1,		    4095,               50,              0,       ref_set_opt},
    {	4,		&g_sys.conf.on_bits,                     1,		    8,                  1,               0,       NULL},
    {	5,		&g_sys.conf.sample_delay,                0,		    1000,               1,               0,       NULL},
    {	6,		&g_sys.conf.sync_period,                 20,		5000,               50,              0,       sync_period_opt},
    {	7,		NULL,			                         0,		    0,                  0,               0,       NULL},
    {	8,		NULL,			                         0,		    0,                  0,               0,       NULL},
    {	9,		NULL,			                         0,		    0,                  0,               0,       NULL},
    {	10,		NULL,			                         0,		    0,                  0,               0,       NULL},
    {	11,		NULL,			                         0,		    0,                  0,               0,       NULL},
    {	12,		NULL,			                         0,		    0,                  0,               0,       NULL},
    {	13,		NULL,			                         0,		    0,                  0,               0,       NULL},
    {	14,		&g_sys.conf.dbg,                         0,		    1,                  0,               0,       NULL},
    {	15,		NULL,			                         9527,		9527,               0,               0,       sys_reset_opt},
};


const sts_reg_map_st status_reg_map_inst[STAT_REG_MAP_NUM]=
{// id			mapped registers																	
    {	0,          &g_sys.stat.sw_ver,						 SOFTWARE_VER},
    {	1,          &g_sys.stat.hw_ver,						 HARDWARE_VER},
    {	2,          &g_sys.stat.sn,						     SERIAL_NO},
    {	3,          &g_sys.stat.man_date,					 MAN_DATE},
    {	4,          &g_sys.stat.ms_status,				     0},
    {	5,          &g_sys.stat.lc_addr,					 0},
    {	6,          NULL,						             0},
    {	8,          NULL,						             0},
    {	9,          NULL,						             0},
    {	10,         NULL,						             0},
    {	11,         NULL,						             0},
    {	12,         NULL,						             0},
    {	13,         NULL,						             0},
    {	14,         NULL,						             0},
    {	15,         NULL,						             0}
};


static void init_load_status(void)
{
    uint16_t i;	
    for(i=0;i<STAT_REG_MAP_NUM;i++)
    {
        if(status_reg_map_inst[i].reg_ptr != NULL)
        {
            *(status_reg_map_inst[i].reg_ptr) = status_reg_map_inst[i].dft;
        }
    }
        
    g_sys.stat.lc_addr = get_local_addr();
    if(g_sys.stat.lc_addr == 0)
    {
        g_sys.stat.ms_status = 1;
    }
    else
    {
        g_sys.stat.ms_status = 2;
    }
        
}

static uint16_t init_load_default(void)
{
    uint16_t i, ret;
    for(i=0;i<CONF_REG_MAP_NUM;i++)		//initialize global variable with default values
    {
        if(conf_reg_map_inst[i].reg_ptr != NULL)
        {
            *(conf_reg_map_inst[i].reg_ptr) = conf_reg_map_inst[i].dft;
        }
    }    
    return ret;
}

void gvar_init(void)
{
    init_load_status();
    init_load_default();
}

/**
  * @brief  write register map with constraints.
  * @param  reg_addr: reg map address.
  * @param  wr_data: write data. 
	* @param  permission_flag:  
  *   This parameter can be one of the following values:
  *     @arg PERM_PRIVILEGED: write opertion can be performed dispite permission level
  *     @arg PERM_INSPECTION: write operation could only be performed when pass permission check
  * @retval 
  *   This parameter can be one of the following values:
  *     @arg 1: write operation success
  *     @arg 0: write operation fail
  */
uint16 reg_map_write(uint16_t reg_addr, uint16_t *wr_data, uint8_t wr_cnt)
{
    uint16_t i;
    uint16_t err_code;
    err_code = REGMAP_ERR_NOERR;		
    
    if((reg_addr+wr_cnt) > CONF_REG_MAP_NUM)	//address range check
    {
        err_code = REGMAP_ERR_ADDR_OR;
        rt_kprintf("MB_SLAVE REGMAP_ERR_ADDR_OR1 failed\n");
        return err_code;
    }
    
    for(i=0;i<wr_cnt;i++)										//min_max limit check
    {
        if((*(wr_data+i)>conf_reg_map_inst[reg_addr+i].max)||(*(wr_data+i)<conf_reg_map_inst[reg_addr+i].min))		//min_max limit check
        {
            err_code = REGMAP_ERR_DATA_OR;
            rt_kprintf("REGMAP_ERR_WR_OR03 failed\n");
            return err_code;	
        }

        if(conf_reg_map_inst[reg_addr+i].chk_ptr != NULL)
        {
            if(conf_reg_map_inst[reg_addr+i].chk_ptr(*(wr_data+i))==0)
            {
                err_code = REGMAP_ERR_CONFLICT_OR;
                rt_kprintf("CHK_PTR:REGMAP_ERR_WR_OR failed\n");
                return err_code;	
            }
        }
    }
    
    for(i=0;i<wr_cnt;i++)										//data write
    {
        if(conf_reg_map_inst[reg_addr+i].reg_ptr != NULL)
            *(conf_reg_map_inst[reg_addr+i].reg_ptr) = *(wr_data+i);//write data to designated register
    }	
    return err_code;		
}

/**
  * @brief  read register map.
  * @param  reg_addr: reg map address.
	* @param  *rd_data: read data buffer ptr.
  * @retval 
  *   This parameter can be one of the following values:
  *     @arg 1: write operation success
  *     @arg 0: read operation fail
  */
uint16 reg_map_read(uint16_t reg_addr,uint16_t* reg_data,uint8_t read_cnt)
{
    uint16_t i;
    uint16_t err_code;
    err_code = REGMAP_ERR_NOERR;
    if((reg_addr&0x8000) != 0)
    {
        reg_addr &= 0x7fff;
        if(reg_addr > STAT_REG_MAP_NUM)	//address out of range
        {
            err_code = REGMAP_ERR_ADDR_OR;
        }
        else
        {
            for(i=0;i<read_cnt;i++)
            {
                *(reg_data+i) = *(status_reg_map_inst[reg_addr+i].reg_ptr);//read data from designated register
            }
        }
    }
    else
    {
        reg_addr = reg_addr;
        if(reg_addr > CONF_REG_MAP_NUM)	//address out of range
        {
            err_code = REGMAP_ERR_ADDR_OR;
        }
        else
        {
            for(i=0;i<read_cnt;i++)
            {
                *(reg_data+i) = *(conf_reg_map_inst[reg_addr+i].reg_ptr);//read data from designated register						
            }
        }		
    }	
    return err_code;
}

/**
  * @brief  show register map content.
  * @param  reg_addr: reg map address.
	* @param  *rd_data: register read count.
  * @retval none
  */
static void rd_reg_map(int argc, char**argv)
{
    uint16_t reg_buf[32];
    uint16_t i;
    uint16_t reg_addr = atoi(argv[1]);
    uint16_t reg_cnt = atoi(argv[2]);
    
    if (argc < 3)
    {
        rt_kprintf("Please input'wr_reg_map <reg_addr> <reg_val>'\n");
        return;
    }
    
    reg_map_read(reg_addr, reg_buf, reg_cnt);
    rt_kprintf("Reg map info:\n");
    for(i=0;i<reg_cnt;i++)
    {
        rt_kprintf("addr:%d;data:%x\n", (reg_addr+i)&0x3fff,reg_buf[i]);
    }
}

static void wr_reg_map(int argc, char**argv)
{
    uint16_t wr_data;
    if (argc < 3)
    {
        rt_kprintf("Please input'wr_reg_map <reg_addr> <reg_val>'\n");
        return;
    }
    
    wr_data = atoi(argv[2]);
    
    reg_map_write(atoi(argv[1]),&wr_data,1);
}

MSH_CMD_EXPORT(wr_reg_map, write data into conf registers.);
MSH_CMD_EXPORT(rd_reg_map, show registers map.);
