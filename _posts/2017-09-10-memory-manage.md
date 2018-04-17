---
layout: post
title: "iOS的内存管理"
date: 2017-09-10 21:13:00.000000000 +08:00
tags: 内存管理
---


> 一些有关iOS的内存管理的认识记录

## 1、copy 关键字

> copy关键字有以下两种使用情况

*   对非集合类对象的 copy 操作及 mutableCopy 操作
*   对集合类对象的 copy操作及 mutableCopy 操作

下面分别对这两种情况举例说明：

1、对**非集合类**对象的 copy 操作及 mutableCopy 操作

```
NSMutableString *mStr = [[NSMutableString alloc] initWithString:@"原可变字符串"];
NSString *copyStr = mStr.copy;       //  --->  1
NSLog(@"mStr:%@ mStr->P:%p",mStr,mStr);
NSLog(@"copyStr:%@ copyStr->P:%p",copyStr,copyStr);
```
此时的执行结果为：
```
mStr:原可变字符串 mStr->P:0x6040002597d0
copyStr:原可变字符串 copyStr->P:0x604000259ad0
```
由mStr与copyStr地址不同，可见，对可变字符串执行 copy 操作时，会对可变字符串（非集合类型）对象执行完全拷贝，即深拷贝。
而此时，如果对 mStr 执行 -appendString: 操作也不会对 copyStr 的值产生影响。同理，将 “1”处改为 mutableCopy也是同样的结论。

2、对**集合类**对象的 copy 操作及 mutableCopy 操作
Foundation框架常用的集合类对象有 NSArray、NSDictionary、NSSet 以及其对应的可变集合等，以 NSArray 举例：
```
NSArray *array = @[@"a", @"b"];
NSArray *copyArray = array.copy;
NSMutableArray *mCopyArray = array.mutableCopy;

NSLog(@"array->P:%p",array);
NSLog(@"copyArray->P:%p",copyArray);
NSLog(@"mCopyArray->P:%p",mCopyArray);
```
此时的执行结果为：
```
array->P:0x600000229ca0
copyArray->P:0x600000229ca0

mCopyArray->P:0x600000244920
```
由此可见，在对 array 执行 copy 操作后，此时的copyArray拥有该array对象的所有权，指针地址自然指向同一个对象，因此**对集合类对象执行copy进行了指针拷贝，也就是浅拷贝**。
但是对于mutableCopy操作则就不一样了，mCopyArray 和 array 的地址是不同的，说明此时执行的是内容拷贝，也即是深拷贝。
但对于集合内的元素来说执行的是什么类型的 copy 呢？
下面我们对上例稍加改造：
```
NSArray *array = @[@"a"];
NSArray *copyArray = array.copy;
NSMutableArray *mCopyArray = array.mutableCopy;

NSLog(@"array->sub_p:%p",array.firstObject);
NSLog(@"copyArray->sub_p:%p",copyArray.firstObject);
NSLog(@"mCopyArray->sub_p:%p",mCopyArray.firstObject);
```
此时的执行结果为：
```
array->sub_p:0x1063d5a90
copyArray->sub_p:0x1063d5a90
mCopyArray->sub_p:0x1063d5a90
```
此时无论是执行 copy 还是 mutableCopy 操作，其集合内的元素的地址全都相同，由此可见，**对集合对象执行 mutableCopy 时，只是对对象本身进行了深拷贝，而集合对象中的元素仍为浅拷贝**。

####总结
>   * 在非集合对象中，无论是 copy 还是 mutableCopy ，都会对原对象进行深拷贝；
>   * 在集合类对象中，对 immutable 对象进行 copy，是指针复制， mutableCopy 是内容复制。对 mutable 对象进行 copy 和 mutableCopy 都是内容复制。但是，集合对象的内容复制仅限于对象本身，对象元素仍然是指针复制。

## 2、为什么使用 copy 关键字而不是 strong 呢？
1、因为父类指针可以指向子类对象，使用 copy 的目的是为了让本对象的属性不受外界影响，使用 copy 后无论传入的是一个可变对象还是不可对象，其本身持有的就是一个不可变的副本。
2、而 strong 则不同， strong 操作会使新属性指向一个可变对象，如果这个可变对象在外部遭到修改,那么将会影响直接该属性。

## 3、weak 关键字

#### 3.1、由**对象所有权**说起

在 OC 中，对内存管理的方式即是对象的引用计数，而内存管理更是成为了面试 iOS 工程师的必备问题之一，几乎成为了两个 iOS程序员见面相互寒暄的一种方式。

众所周知，对于一个实例对象来说，每次对该对象执行一次强引用操作就会使其引用计数+1，而一次释放（release）操作又会使其引用计数-1，当一个对象的引用计数值变为0时，需要对该实例对象的内存地址进行回收。概念可以说非常简单，但是**对象的引用计数存放的位置**在哪里呢？如果你认为一个对象的引用计数存放在该对象的内存块里，并有对象内部的实例变量所记录那就错了。其实，对象的引用计数由专门的**引用计数表**所记录，在引用记录表中存放着各个**内存块的地址**以及其**对应的引用计数**，这样的方式有两种好处：
* 对象使用的内存块的分配无需考虑实例对象内部；
* 引用计数表存放有内存地址，通过引用计数表，可以追溯到个对象的内存块。

那么什么是对象的所有权呢？例如：
```
id obj = [[NSObject alloc] init];
```
上面几句代码，NSObject类对象通过其+ alloc类方法开辟了一段内存空间并创建了一个不知道叫什么名字的实例对象（为方便理解，不妨把这个不知道叫什么的匿名实例对象代称为：A），接着又调用 A 对象的 init 方法对自己进行了初始化操作，到此为止，一个全新的 A 对象已经诞生了，但是我们没法进一步使用它，因为在 ARC 的内存管理方式里面，如果 A 仅仅创建完成并没有别的变量**拥有**它的话，那么该匿名对象会在一个 Runloop 中被自动释放掉。而这里的**拥有**，指的便是拥有对象的所有权，正如上例中将 A 赋值给一个 id 类型的 obj 变量那样。此时，可以说 obj 变量已经持有了自己（这里指 A）生成的对象，也拥有了其所有权。

当一个变量拥有了某个对象的所有权后，对于该变量有一下几种说明：
* 如果某个实体拥有一个对象，则该实体要负责确保对其拥有的对象进行清理；
* 这个实体可以是另一个对象，或者一个函数；
* 多个实体可以同时拥有同一个特定对象。

**其实，明白这些并没有什么卵用！** OC 的内存管理只需要理解四句话就行了：
* 自己生成的对象，自己持有；**1**
* 非自己生成的对象，自己也能持有；**2**
* 不再需要自己持有对象时释放；**3**
* 非自己持有的对象无法释放。**4**

对于**1**，这里不需要做过多的解释，看看上面那一句代码就明白了，这里只需要特别说明的是，在 Foundation 框架中，凡是使用**alloc**、**new**、**copy**、**mutableCopy**关键字生成的对象，都为自己生成并持有的对象；

对于**2**来说，请看代码：
```
id obj = [NSMutableArray array];
```
上面NSMutableArray产生的对象赋值给变量 obj ，但此时 obj 并不持有该对象，不过可以使用关键字**retain**来持有对象；

对于**3**来说就比较好理解了，**1**中自己持有的对象已经使用完毕，不再需要的时候，即可使用关键字**release**释放，而释放的前提是：
* 该对象已经确定不再使用；
* 确定拥有该对象的所有权。
满足以上两个条件，则就是像**4**中所说的非自己持有的对象无法释放。

#### 3.2、weak 关键字的实现方式

上面叨逼叨了这么对简直没个卵用，因为在 ARC 环境中，编译器会自动帮助我们管理对象的生命周期，其实现的方式虽然也是在代码中插入这些内存管理的语句，但这极大的降低了我们手动管理内存的难度，也提高了开发效率。
但是前面也说了，内存管理是面试iOS 程序员的时候一种打招呼的方式，总不能别人给你打了招呼你却傲娇的不去回应。那么此时，还是有必要对 weak 关键字进行一些说明的，此外，掌握了 weak 的特性之后，我们可以脑洞打开的做一些骚操作，详情请看**3.3**。

weak 关键字较为常用的用途是用来解决循环引用造成的对象无法释放的问题，回到最开始的那一句代码：
```
id  obj = [[NSObject alloc] init];
```
前面已经说过，此时的 obj 变量拥有匿名对象 A 的所有权，也持有该对象，这是因为编译器会在 obj 前面加上一个默认的关键字，如下：
```
id __strong obj = [[NSObject alloc] init];
```
然而，如果此时把**__strong**改为**__weak**时会怎样呢？前面也已经给出了答案，因为没有任何对象会拥有匿名对象 A，所有 A 会在一个 Runloop 时内释放掉，所有如果强行将上面一句改为下面的这种形式：
```
id __weak obj = [[NSObject alloc] init];
```
此时编译器会给出警告：
> Assigning retained object to weak variable; object will be released after assignment

那么我们如何使用呢？如下：
```
id obj = [[NSObject alloc] init];
id __weak obj1 = obj;
```
此时的 obj1已经被赋值为 obj，但却不持有该对象，也就是说 obj 的引用计数并不会增加，而我们在该作用域中也可以放心大胆的使用 obj1 并且不会产生循环引用。

说到这里，你会发现，你已经遇到了面试攻略上所说的**weak 修饰的变量在其引用的对象被废弃时，则会将 nil 赋值给该变量**以及**weak 修饰的变量将会被注册到autoreleasepool 中**。

**这些像黑魔法一样的功能是怎么实现的呢？**

其实，weak 关键字修饰的变量会被编译器自动添加到 autoreleasepool 中，并且会将其持有的对象的地址指针作为 key，对象的值作为 value 存储到专门的 weak 哈希表中，一旦其所持有的对象被废弃后，当一个 Runloop 到来时，系统会自动去哈希表中，以该废弃的对象的地址作为 key，查找所有记录，并将找到地址的 value 设置为 nil，再去引用计数表中删除该废弃对象的地址为健值的记录。

#### 3.3 利用 weak 关键字实现一个骚操作

说了这么多，也即是说，weak 不会真正的持有对象的所有权，而且当其指向的对象被废弃时，weak 所修饰的变量也会被自动赋值为 nil，从而被系统回收。

在开发中，我们通常会使用单例模式，但是单例会有一个不好的问题就是，**在整个程序的运行周期中，单例对象都不会被释放，从而会对内存造成一定的影响**，那么我们可以利用 weak 关键字对单例模式进行改造，达到**如果单例对象被外部持有，则永远不会被释放，一旦不被外部持有，则会在 Runloop 时被回收内存**的目的。

以名为CPFWeakSingleton的类名为例，代码如下：
```objc
@implementation CPFWeakSingleton

+ (instancetype)sharedInstacne {
return [[self alloc] init];
}

+ (instancetype)allocWithZone:(struct _NSZone *)zone {
static __weak CPFWeakSingleton *weakInstance;
CPFWeakSingleton *strongInstance = weakInstance;
@synchronized(self) {
if (strongInstance == nil) {
strongInstance = [super allocWithZone:zone];
weakInstance = strongInstance;
}
}
return strongInstance;
}
@end
```
下面对其进行验证：
```
_strongInstance = [CPFWeakSingleton sharedInstacne];
NSLog(@"1---%p",_strongInstance);
_strongInstance.testStr = @"保留所有权";
NSLog(@"2---%p",_strongInstance);

sleep(5);
NSLog(@"3---%p",[CPFWeakSingleton sharedInstacne]);
```
运行结果如下：
```
1---0x604000202ea0
2---0x604000202ea0
3---0x604000202ea0
```
可以看出，当我们通过_strongInstance变量持有单例对象时，在经过 Runloop 之后，单例对象也不会被释放（sleep函数是为了验证 Runloop 后对象是否会被回收）。

然而我们对上例稍加改动，使_strongInstance被释放后会发生什么呢？
```
_strongInstance = [CPFWeakSingleton sharedInstacne];
NSLog(@"1---%p",_strongInstance);
_strongInstance.testStr = @"保留所有权";
NSLog(@"2---%p",_strongInstance);

_strongInstance = nil;

sleep(5);
NSLog(@"3---%p",[CPFWeakSingleton sharedInstacne]);
```
此时的运行结果如下：
```
1---0x600000009430
2---0x600000009430
3---0x604000007570
```
可以看出，当外部的_strongInstance对象被释放，不再持有单例对象的时候，或者超出此时单例对象的作用域时（上述代码未演示），该单例对象也会在 Runloop 中被系统回收，当我们再次使用sharedInstacne类方法获取单例对象的时候，则会创建一个新的单例对象。这样，就能即使用单例，又解决了产生的单例对象一直占用内存资源，而且在整个程序的运行周期内都不会被释放的问题。

-EOF-
