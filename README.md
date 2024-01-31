# device-mapper-proxy
Модуль ядра linux, реализующий цель для device mapper. Собирает статистику о блочных I/O операциях в sysfs.

## Зависимости
! модуль разрабатывался и тестировался на Fedora с ядром версии 6.6.13-200.fc39.x86_64 !
```
dnf install kernel-devel-6.5.6-300.fc39.x86_64
```

## building
```
git clone https://greg-zamza/device-mapper-proxy
cd device-mapper-proxy
make
```

## usage
Следующие команды требуют прав суперпользователя (su/doas/sudo).

Загрузите модуль в ядро
```
insmod dmp.ko
```

Убедитесь, что модуль загружен
```
lsmod | grep dmp
```

Выберите или создайте блочное устройство, с которым будет связано виртуальное БУ. Например:
```
dmsetup create zero1 --table "0 2560 zero"
```

Создайте виртуальное блочное устройство, замапленное на устройство из предыдущего шага.
NB! Первые два аргумента в --table (первый сектор и количество секторов) должны совпадать с таковыми у устройство, с которым мы связываем виртуальное БУ.
```
dmsetup create dmp1 --table "0 2560 dmp /dev/mapper/zero1"
```
После создания устройства в sysfs появился файл `/sys/kernel/dmp/volumes`, в котором находится статистика.

## Тестирование
```
dd if=/dev/random of=/dev/mapper/dmp1 bs=4k count=10
dd if=/dev/mapper/dmp1 of=/dev/bull bs=4k count=10
cat /sys/kernel/dmp/volumes
```
Также во время выполнения блочных I/O операций можно посмотреть логи:
```
dmesg -wH
```

## Отключение модуля
```
dmsetup remove dmp1 zero1
rmmod dmp
```
После отключения модуля файл `/sys/kernel/dmp/volumes` будет удалён.



