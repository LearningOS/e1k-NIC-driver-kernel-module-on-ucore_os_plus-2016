对13和14的工作进行了拆分，编译时可选择swap或fat或全不选(即原来的初始模式)
运行选择不同的shell即可对应的正确结果
1.uCore_run.fat 只选fat
2.uCore_run.os	全不选
3.uCore_run.swap 只选swap
4.uCore_run		全选

todo: 代码merge，问题1.swap修改过大
					2.fs被删
					3.fs module和本身fs有很大重复