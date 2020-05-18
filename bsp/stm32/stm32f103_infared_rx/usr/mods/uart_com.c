#include <rtthread.h>
#include "board.h"
#include "drv_usart.h"

#define MAX_PKG_LEN             32
//#define RS485_TE                GET_PIN(B, 12)
#define SAMPLE_UART_NAME       "uart3"

/* ���ڽ�����Ϣ���ź��� */
static struct rt_semaphore rx_sem;
static rt_device_t serial;

static void rs485_pin_init(void)
{
    rt_pin_mode(RS485_TE,PIN_MODE_OUTPUT);    
    rt_pin_write(RS485_TE,PIN_LOW);    
    rt_kprintf("com dir pin initialized!\n");
}

void com_tx(uint8_t *src_ptr, uint16_t cnt)
{
    rt_pin_write(RS485_TE,PIN_HIGH);
    rt_device_write(serial, 0, src_ptr, cnt);
    rt_pin_write(RS485_TE,PIN_LOW);
}

/* �������ݻص����� */
static rt_err_t uart_rx_ind(rt_device_t dev, rt_size_t size)
{
    /* ���ڽ��յ����ݺ�����жϣ����ô˻ص�������Ȼ���ͽ����ź��� */
    if (size > 0)
    {
        rt_sem_release(&rx_sem);
    }
    return RT_EOK;
}

static char uart_sample_get_char(void)
{
    char ch;

    while (rt_device_read(serial, 0, &ch, 1) == 0)
    {
        rt_sem_control(&rx_sem, RT_IPC_CMD_RESET, RT_NULL);
        rt_sem_take(&rx_sem, RT_WAITING_FOREVER);
    }
    return ch;
}

/* ���ݽ����߳� */
static void data_parsing(void)
{
    char ch;
    char data[MAX_PKG_LEN];
    static char i = 0;

    while (1)
    {
        ch = uart_sample_get_char();
        if(ch == '\n')
        {
            data[i++] = '\0';
            rt_kprintf("data=%s\r\n",data);
            com_tx((uint8_t*)data,i);
            i = 0;
            continue;
        }
        i = (i >= MAX_PKG_LEN-1) ? MAX_PKG_LEN-1 : i;
        data[i++] = ch;
    }
}


static int com_app(void)
{
    rt_err_t ret = RT_EOK;
    char uart_name[RT_NAME_MAX];
    char str[] = "COM_APP started!\r\n";    
    rs485_pin_init();
    rt_strncpy(uart_name, SAMPLE_UART_NAME, RT_NAME_MAX);

    /* ����ϵͳ�еĴ����豸 */
    serial = rt_device_find(uart_name);
    if (!serial)
    {
        rt_kprintf("find %s failed!\n", uart_name);
        return RT_ERROR;
    }

    /* ��ʼ���ź��� */
    rt_sem_init(&rx_sem, "rx_sem", 0, RT_IPC_FLAG_FIFO);
    /* ���жϽ��ռ���ѯ����ģʽ�򿪴����豸 */
    rt_device_open(serial, RT_DEVICE_FLAG_INT_RX);
    
//    rt_device_control(serial, RT_DEVICE_CTRL_CONFIG, &config);
    /* ���ý��ջص����� */
    rt_device_set_rx_indicate(serial, uart_rx_ind);
    /* �����ַ��� */
    com_tx((uint8_t*)str,(sizeof(str) - 1));
//    rt_device_write(serial, 0, str, (sizeof(str) - 1));

    /* ���� serial �߳� */
    rt_thread_t thread = rt_thread_create("serial", (void (*)(void *parameter))data_parsing, RT_NULL, 1024, 25, 10);
    /* �����ɹ��������߳� */
    if (thread != RT_NULL)
    {
        rt_thread_startup(thread);
    }
    else
    {
        ret = RT_ERROR;
    }

    return ret;
}
/* ������ msh �����б��� */
//INIT_APP_EXPORT(com_app);
//MSH_CMD_EXPORT(uart_init, uart device sample);
