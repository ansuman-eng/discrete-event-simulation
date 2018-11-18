import numpy as np
beta=10
process_count=10
a=np.round(np.random.exponential(scale=beta,size=(process_count,2)))
np.savetxt('process.csv',a,delimiter=",",fmt="%i")
