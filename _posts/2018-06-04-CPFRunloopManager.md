---
layout: post
title: "基于Runloop的自定义主线程任务调度"
date: 2018-06-04 16:13:00.000000000 +08:00
tags: Runloop
---

### 前言
前段时间工作不忙，深入学习了一些 Runloop 相关的知识。在学习和研究的过程中，也对网上相互抄来抄去的知识点有了一些透彻的认识，并且编写了一套简陋的代码，命名为 **CPFRunloopTaskManager**，用于解决一些必须在主线程处理的任务又要防止页面卡顿的问题。现开篇记录一下。

项目地址：[https://github.com/githubError/MessyRepository/tree/master/CPFRunloopDemo](CPFRunloopTaskManager)

### 有关Runloop的认识
1. Runloop 与 线程 之间存在一一对应的关系，这种对应关系在 Runloop 的创建之初便已经建立；
2. Runloop 的包含多个Model，每个Model中又包含多个输入源 Timer、Source、Observer等
3. Runloop 中必须包含至少一个输入源，即timer、source0、source1，否则Runloop会直接退出；
4. 在子线程中创建的Runloop，在Runloop退出之前，必须手动添加一个timer、source0、source1来保持其处于活跃状态；
5. 通过performSelector:onThread:，快速为 Runloop 添加一个source0，进而唤醒Runloop；
6. 通过performSelector:withObject:afterDelay:，快速为Runloop添加一个timer，进而唤醒Runloop；

### Runloop的输入源
Runloop的输入源是Runloop所处理的事件源之一，主要用于线程或进程之间的数据传递和消息通知。输入源分为**基于端口输入源（Source1）** 和 **基于非端口输入源（Source0）**

其中，Source0和Source1都属于CFRunloopSourceRef类型，初配置方式不同外，这两种输入源与线程或进程的交互形式也有所不同：
- 基于端口输入源（Source1），监听端口，当端口有消息到达时，相应的Source1就会被触发回调，完成相应的操作。
- 基于非端口输入源（Source0），不监听端口，让Source0执行其回调，只需要手动标记Source0为待处理状态，然后唤醒Source0所在的Runloop即可。

##### 创建Source0
```objc
// 创建基于非端口的source0
// 创建source0上下文，context是一个结构体，其中的info为携带的内容，可传递给回调方法perform
CFRunLoopSourceContext context = {.version = 0, .info = (__bridge void *)self, .perform = perform};
// 创建source0
CFRunLoopSourceRef source0 = CFRunLoopSourceCreate(kCFAllocatorDefault, 0, &context);
// 添加source0到相应的RunloopMode
CFRunLoopAddSource(CFRunLoopGetCurrent(), source0, kCFRunLoopDefaultMode);

// 标记source为待处理事件源
CFRunLoopSourceSignal(source0);
// 唤醒所在的Runloop
CFRunLoopWakeUp(CFRunLoopGetCurrent());
```


回调：
```objc
static void perform (void * info) {
    NSLog(@"++++++ %@",info);
}
```

##### 创建Source1
```objc
// 创建基于端口的source1
// 创建接收端口，用于监听端口，其中callBack为回调方法，类型为CFMessagePortCallBack
CFMessagePortRef localPort = CFMessagePortCreateLocal(kCFAllocatorDefault, CFSTR(), callBack, NULL, NULL);
// 创建端口输入源
CFRunLoopSourceRef source = CFMessagePortCreateRunLoopSource(kCFAllocatorDefault, localPort, 0);
// 添加端口输入源
CFRunLoopAddSource(CFRunLoopGetCurrent(), source, kCFRunLoopDefaultMode);


// 创建发送端口， 发送下面的消息体
CFMessagePortRef remotePort = CFMessagePortCreateRemote(kCFAllocatorDefault, CFSTR());
// 创建消息体
NSString *message = @"this is a message";
NSData *messageData = [message dataUsingEncoding:NSUTF8StringEncoding];
CFDataRef messageDataRef = CFBridgingRetain(messageData);

// 发送消息
CFMessagePortSendRequest(remotePort, kCFMessagePortSuccess, messageDataRef, 0, 10, kCFRunLoopDefaultMode, NULL);
```

回调（类型为CFMessagePortCallBack，有如下几个参数和返回值）：

```objc
static CFDataRef callBack (CFMessagePortRef local, SInt32 msgid, CFDataRef data, void *info) {

NSData *messageData = (__bridge NSData *)data;
NSString *message = [[NSString alloc] initWithData:messageData encoding:NSUTF8StringEncoding];
NSLog(@"++-----++ %@",message);
return NULL;
}
```



### RunloopTaskManager
> 支持创建task任务，已添加的task将会在主线程Runloop进入休眠之前被执行，并且每次Runloop只执行一个task，达到必须在主线程执行的繁重的任务延后执行的目的，经过测试，在频繁快速的滚动UITableView的时候，屏幕的刷新率仍能保持在56FPS以上，对防止屏幕卡顿掉帧有良好的效果。

### CPFRunloopTaskManager

- **CPFRunloopTaskManager** 是一个单例，通过几个实例方法添加或者移除任务，默认最多添加10个任务。

```
+ (instancetype)defaultManager;
```

- 需要注意的是，移除任务必须明确某个任务对象，或者指定任务的唯一标识 **identifier**，且正在执行的任务无法移除。

```
- (void)addTaskUnit:(CPFRunloopTaskUnit *)taskUnit;
	
- (void)removeTaskUnit:(CPFRunloopTaskUnit *)taskUnit;

- (void)addTask:(CPFRunloopTask)task forIdentifier:(NSString *)identifier;
- (void)removeTaskForIdentifier:(NSString *)identifier;

- (void)removeAllTaskUnit;

```
- 已添加的任务支持暂停和恢复执行，正在执行的任务无法暂停。

```
- (void)suspend;
- (void)resume;
```

- 支持通过为主线程Runloop添加一个Source0的方式，来立即出发一个新任务。

```
- (void)executeTask:(CPFRunloopTask)task;
```

### CPFRunloopTask

- CPFRunloopTaskManager 中添加的任务，都是 **CPFRunloopTaskUnit** 实例对象。
- CPFRunloopTaskUnit 对象通过指定初始化方法 *-initTaskUnit:forIdentifier:*  创建。第一个参数是一个返回布尔值的Block，用来包裹要执行的任务，任务执行结束返回Yes，否则返回No；第二个参数是任务的唯一标识符，用于移除任务。


### 用法

- 添加任务

```
CPFRunloopTaskUnit *taskUnit_1 = [[CPFRunloopTaskUnit alloc] initTaskUnit:^BOOL{
        NSLog(@"正在执行 taskUnit_1");
        return YES;
    } forIdentifier:@"taskUnit_1"];
[[CPFRunloopTaskManager defaultManager] addTaskUnit:taskUnit_1];
```

- 暂停任务

```
[[CPFRunloopTaskManager defaultManager] suspend];
```

- 移除任务

```
[[CPFRunloopTaskManager defaultManager] removeTaskForIdentifier:@"taskUnit_4"];
```

### 关于Demo

Demo 中使用一个UITableView模拟任务的频繁触发，在Cell将要出现的时候添加绘制任务，在Cell消失的时候移除绘制的视图。CPFRunloopTaskManager会在UITableView停止滚动的时候开始顺序执行添加的任务。每个任务花费一个Runloop的循环，防止任务卡顿主线程，通过这种延后执行的方式提高屏幕刷新率。
