# SimpleSmartWatch
a Simple Smart Watch
菜单分为主菜单（即界面交互），时间设置菜单与调节菜单 - 时间设置。应用菜单 - 应用调节。
切换菜单采用OLED 双缓冲Frontbuffer和Bakcbuffer来提前渲染。Frontbuffer为DMA搬运OLED显示。Backbuffer来CPU渲染。
采用双向链表设计将菜单设置为可往返的一串可循环的链表
流程循环
1. 应用层调用：
   OLED_NewFrame()      → 清空 back_buffer
   
2. 调用多个绘制函数：
   OLED_DrawLine()        → 画到 back_buffer
   OLED_PrintString()     → 写文字到 back_buffer
   
3. 画完后调用：
   OLED_ShowFrame()          → 交换缓冲区 + 启动 DMA
   
4. DMA发送：
   front_buffer → I2C → OLED
   
5. 同时CPU可以继续：
   OLED_DMA_Frame()
   绘制下一帧...
   
6. DMA 完成后中断：
   清除 dma_busy 标志

   启动第0页 DMA → 传输完成 → 中断发送 → 释放信号量 →  busy通过
   
1. 先绘制数据到 back_buffer
2. 等待上次DMA完成（dma_busy=0）
3. 交换指针（调动数组）
4. 开始DMA发送（dma_busy=1）
5. DMA发送中...
6. DMA完成 → 中断回调 → dma_busy=0
7. 回到步骤1

--菜单上的使用
枚举菜单节点
使用之前的结构体来初始化所有菜单，将其一一添加进链表

#日志
 -  1 开始RTOS项目实战，用RTOS写了两个轮询任务。后更改为队列发送。实现双按键发送串口通信
 -  2 目的让OLED亮起，利于If else switch case做了4个菜单用于检验切换。实现RTC实时时钟，时钟能运行但OLED上无法是实时显示，检测是OLED屏幕没用进行实时刷新，定义变量标志和HAL_GETTICK检测当前时间用于每次刷新。
 -  3 时间会在突然间快1s，但不影响实际计时，发现是OLED与RTC抢夺任务，加入互锁。加入编码器，预计实现编码器用来更改时间，长按进入，短按进入调节，旋转调节。freertos.c代码太多，重构模块化，代码出现紊乱，重新构造项目
 -  4 编码器加入。取消之前两个轮询任务，尝试二进制信号量发送，失败。利用中断记录+消抖完成长按与短按。建立结构体设置菜单模式。在显示任务中利用队列通知和按键中断实现菜单来回之间切换。
 -  5 OLED切换菜单速度太慢，准备采用DMA+双缓冲的策略。重写OLED.c，首先OLED.send加入中断发送,另外设置信号量,加入缓冲数组和数组指针,在oledshowframe函数中进行交换sned发送的数组，因为采用OLED，DMA分页发送，硬件CH1116限制原因无法自增地址只能1页1页发送，每次发送触发中断回调时释放信号量。此时闪屏
 -  6 发现是各DMA数据被抢占，添加busy标志，删除信号量，在frame出设置busy为1代表此时dma传输中，在oledshow中若busy为1则阻塞，在中断回调中运输完毕busy为0.此时黑屏，发现1是清屏必须阻塞清屏，因为分页发送，实际上send没用了，在show里完成所有功能，并且show只发送一页，剩下的在中断中发送。
 -  7 定义两个指针current和display_start实现滑动窗口，current控制折线框移动，displaystart控制菜单移动，只有current到达边缘时，displaystart才会刷新，在时间页面跳过，因为时间不参与折线画画
 -  8 优化菜单切换动画。原 Menu_Switch 函数在外部手动切换 current 后调用，方向计算及窗口同步失效。最终拆分为 Menu_switch_Left / Right，由调用侧明确指定滑入方向，内部原子化完成 current 切换 + 动画启动。修复 display.c 动画分支中括号导致的绘制遗漏，时间页面切换动画接入完成。
