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
