首先通过课程视频介绍，聚集是通过hash来提高效率，外部的hash,将哈希表放到磁盘。先过滤再hash.分别放到不同的桶里面。第二步时rehash，一个hash可能产上碰撞，对第一次发生碰撞的在进行hash。去重复可以先去重再hash，也可以hash再去重。求平均的值的话，记录个数和总和值。