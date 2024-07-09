import matplotlib.pyplot as plt
data =  [0.4, 0.45, 0.45, 0.48, 0.5]
data1 = [0.65, 0.7, 0.7, 0.74, 0.8]


Distance  = [30, 60, 90, 120, 150]

# Plotting
plt.figure(figsize=(12, 6))
plt.plot(Distance, data, linestyle='-', color='b', label='1 byte packet')
plt.plot(Distance, data1, linestyle='-', color='r', label='256 byte packet')
plt.title('Latency of LoRA communication')
plt.xlabel('Distance (m)')

plt.yticks([0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1], 
           ['0', '10', '20', '30', '40', '50', '60', '70', '80', '90', '100'])
plt.ylabel('Time (ms)')
plt.grid(True)
plt.legend()
plt.tight_layout()

# Display the plot
plt.show()