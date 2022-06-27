from knn import calculate_distances,majority_voting,knn,split_train_and_validation,cross_validation
from kmeans import centroidInıtialization,assign_clusters,calculate_cluster_centers,kmeans
from hac import find_min_indexes,single_linkage,complete_linkage,average_linkage,centroid_linkage,hac
import numpy as np
import matplotlib.pyplot as plt



kmeanDS1 = np.load("hw2_material/kmeans/dataset1.npy")
kmeanDS2 = np.load("hw2_material/kmeans/dataset2.npy")
kmeanDS3 = np.load("hw2_material/kmeans/dataset3.npy")
kmeanDS4 = np.load("hw2_material/kmeans/dataset4.npy")

train_set = np.load("hw2_material/knn/train_set.npy")
train_labels = np.load("hw2_material/knn/train_labels.npy")
test_set = np.load("hw2_material/knn/test_set.npy")
test_labels = np.load("hw2_material/knn/test_labels.npy")

hacDS1 = np.load("hw2_material/hac/dataset1.npy")
hacDS2 = np.load("hw2_material/hac/dataset2.npy")
hacDS3 = np.load("hw2_material/hac/dataset3.npy")
hacDS4 = np.load("hw2_material/hac/dataset4.npy")

'''
print("## KNN Algorithm ##")
#Fold CV accuracy graphic

print("## L1 Manhattan Distance ##")
xPts = []
yPts = []
for i in range(1,180): # for k = 1,2,3 ......179
    accuracy = cross_validation(train_set,train_labels,10,i,'L1')
    xPts.append(i)
    yPts.append(accuracy)
xPts = np.array(xPts)
yPts = np.array(yPts)

plt.plot(xPts,yPts)
plt.show()

#Test accuracy for best k

bestkVal = xPts[np.argmax(yPts)]
accuracy = knn(train_set,train_labels,test_set,test_labels,bestkVal,'L1')
print ("Best accuracy for k = " ,bestkVal,"is",accuracy)

print("## L2 Euclidean Distance ##")
xPts = []
yPts = []
for i in range(1,180): # for k = 1,2,3 ......179
    accuracy = cross_validation(train_set,train_labels,10,i,'L2')
    xPts.append(i)
    yPts.append(accuracy)
xPts = np.array(xPts)
yPts = np.array(yPts)

plt.plot(xPts,yPts)
plt.show()

#Test accuracy for best k

bestkVal = xPts[np.argmax(yPts)]
accuracy = knn(train_set,train_labels,test_set,test_labels,bestkVal,'L2')
print ("Best accuracy for k = " ,bestkVal,"is",accuracy)
'''





'''
print("## KMEANS Algorithm")

print("## Elbow graph ##")
inp = input("Please enter dataset number // e.g for dataset1 enter 1\n")
datasetL = [kmeanDS1,kmeanDS2,kmeanDS3,kmeanDS4]
datasetIndex = int(inp) - 1
centers = []
objectiveFunctions = []
cluster = []
for i in range (1, 11): #k = 1,2,3 .... 10
    cluster.append(i)
    minObjectiveFunc = 99999999
    clusterCenter = np.array([])
    for _ in range (0, 9):
        initial_center = centroidInıtialization(datasetL[datasetIndex],i)
        x, y = kmeans(datasetL[datasetIndex],initial_center)
        if y < minObjectiveFunc:
            minObjectiveFunc = y
            clusterCenter = x
    centers.append(clusterCenter)
    objectiveFunctions.append(minObjectiveFunc)
objectiveFunctions = np.array(objectiveFunctions)
cluster = np.array(cluster)

plt.plot(cluster,objectiveFunctions)
plt.show()

'''




'''
print("## HAC Algorithm ##")

inp = input("Please enter dataset number // e.g for dataset1 enter 1\n")
inp2 = input("Please enter linkage method // e.g for single linkage type single\n" )
datasetL = [hacDS1,hacDS2,hacDS3,hacDS4]
datasetIndex = int(inp) - 1
if (int(inp) != 4):
    if (inp2 == 'single'):
        cluster = hac(datasetL[datasetIndex],single_linkage,2)
        plt.scatter(cluster[1][:,0],cluster[1][:,1],color = 'red')
        plt.scatter(cluster[0][:, 0], cluster[0][:, 1], color='blue')
        plt.show()
    elif (inp2 == 'complete'):
        cluster = hac(datasetL[datasetIndex],complete_linkage,2)
        plt.scatter(cluster[1][:,0],cluster[1][:,1],color = 'red')
        plt.scatter(cluster[0][:, 0], cluster[0][:, 1], color='blue')
        plt.show()
    elif (inp2 == 'average'):
        cluster = hac(datasetL[datasetIndex],average_linkage,2)
        plt.scatter(cluster[1][:,0],cluster[1][:,1],color = 'red')
        plt.scatter(cluster[0][:, 0], cluster[0][:, 1], color='blue')
        plt.show()
    else:
        cluster = hac(datasetL[datasetIndex], centroid_linkage, 2)
        plt.scatter(cluster[1][:, 0], cluster[1][:, 1], color='red')
        plt.scatter(cluster[0][:, 0], cluster[0][:, 1], color='blue')
        plt.show()
else:
    if (inp2 == 'single'):
        cluster = hac(datasetL[datasetIndex],single_linkage,4)
        plt.scatter(cluster[3][:, 0], cluster[3][:, 1], color='green')
        plt.scatter(cluster[2][:, 0], cluster[2][:, 1], color='yellow')
        plt.scatter(cluster[1][:,0],cluster[1][:,1],color = 'red')
        plt.scatter(cluster[0][:, 0], cluster[0][:, 1], color='blue')
        plt.show()
    elif (inp2 == 'complete'):
        cluster = hac(datasetL[datasetIndex],complete_linkage,4)
        plt.scatter(cluster[3][:, 0], cluster[3][:, 1], color='green')
        plt.scatter(cluster[2][:, 0], cluster[2][:, 1], color='yellow')
        plt.scatter(cluster[1][:,0],cluster[1][:,1],color = 'red')
        plt.scatter(cluster[0][:, 0], cluster[0][:, 1], color='blue')
        plt.show()
    elif (inp2 == 'average'):
        cluster = hac(datasetL[datasetIndex],average_linkage,4)
        plt.scatter(cluster[3][:, 0], cluster[3][:, 1], color='green')
        plt.scatter(cluster[2][:, 0], cluster[2][:, 1], color='yellow')
        plt.scatter(cluster[1][:,0],cluster[1][:,1],color = 'red')
        plt.scatter(cluster[0][:, 0], cluster[0][:, 1], color='blue')
        plt.show()
    else:
        cluster = hac(datasetL[datasetIndex], centroid_linkage, 4)
        plt.scatter(cluster[3][:, 0], cluster[3][:, 1], color='green')
        plt.scatter(cluster[2][:, 0], cluster[2][:, 1], color='yellow')
        plt.scatter(cluster[1][:, 0], cluster[1][:, 1], color='red')
        plt.scatter(cluster[0][:, 0], cluster[0][:, 1], color='blue')
        plt.show()


'''
