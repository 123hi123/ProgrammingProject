cmd_/home/evan/CH2/modules.order := {   echo /home/evan/CH2/jiffies_module.ko;   echo /home/evan/CH2/seconds_module.ko; :; } | awk '!x[$$0]++' - > /home/evan/CH2/modules.order
