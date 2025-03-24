cmd_/home/evan/CH2/Module.symvers := sed 's/\.ko$$/\.o/' /home/evan/CH2/modules.order | scripts/mod/modpost -m -a  -o /home/evan/CH2/Module.symvers -e -i Module.symvers   -T -
