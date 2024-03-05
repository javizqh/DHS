import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

plt.style.use('dark_background')

data_sizes = ['10', '100', '1000', '10000']
one_percent = int (4 * 100 * 1000 / 100)

for i in data_sizes:
        fig, ax = plt.subplots()
        ax.set_axisbelow(True)

        data = pd.read_csv('data/' + i + '/ddhs.csv')
        data2 = pd.read_csv('data/' + i + '/fuzzy.csv')
        data3 = pd.read_csv('data/' + i + '/stdc.csv')

        y_label = ['DHS 1% Max','DHS Mean','Fuzzy 1% Max','Fuzzy Mean', 'Std C 1% Max','Std C Mean']
        y_color = ['darkslateblue', 'royalblue', 'darkslateblue', 'royalblue', 'darkslateblue', 'royalblue']

        max_val   = np.argpartition(data['SEARCH_TIME'],  -one_percent)[-one_percent:]
        max_val_2 = np.argpartition(data2['SEARCH_TIME'], -one_percent)[-one_percent:]
        max_val_3 = np.argpartition(data3['SEARCH_TIME'], -one_percent)[-one_percent:]
        performance = [np.mean(data['SEARCH_TIME'][max_val])/1000,      np.mean(data['SEARCH_TIME'])/1000,
                np.mean(data2['SEARCH_TIME'][max_val_2])/1000,   np.mean(data2['SEARCH_TIME'])/1000,
                np.mean(data3['SEARCH_TIME'][max_val_3])/1000,   np.mean(data3['SEARCH_TIME'])/1000]

        ax.xaxis.grid(color='#555555', linewidth=1, linestyle='dashed')
        ax.xaxis.grid(True)
        ax.barh(y_label, performance, align='center', color=y_color)
        ax.set_yticks(y_label, labels=y_label)
        ax.invert_yaxis()  # labels read top-to-bottom
        plt.xlabel('Time (microseconds)')
        plt.title('Search time (' + i + ' words)')
        fig.set_size_inches(10.5, 6.5)
        fig.savefig('result/Graph' + i + '.png', dpi=100)
        # plt.show()