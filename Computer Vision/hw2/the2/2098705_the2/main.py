import cv2 as cv
import numpy as np
from sklearn.cluster import KMeans
import os
dirNames = ['apple','aquarium_fish','beetle','camel','crab','cup','elephant','flatfish','lion','mushroom',
            'orange','pear','road','skyscraper','woman']
fileNames = []

for filename in os.listdir('the2_data/train/apple'):
    fileNames.append(filename)

#creates a dictionary that has keys as object names and values as all desprictors found by SIFT
#in that object class in the train dataset
def SIFTdescriptorsOfTrain():
    objectsAndDescriptorsDict = {}
    for dirName in dirNames:
        descriptorL = np.empty((0, 128))
        for filename in fileNames:
            img = cv.imread('the2_data/train/' + dirName + '/' + filename)
            gray= cv.cvtColor(img,cv.COLOR_BGR2GRAY)
            sift = cv.SIFT_create()
            kp, des = sift.detectAndCompute(gray,None)
            if (len(kp) != 0):
                descriptorL = np.concatenate((descriptorL,des),axis=0)
        objectsAndDescriptorsDict[dirName] = descriptorL
    return objectsAndDescriptorsDict


#creates a dictionary that has keys as object names and values as all desprictors found by Dense-SIFT
#in that object class in the train dataset
def denseSIFTdescriptorsOfTrain(stepSize):
    objectsAndDescriptorsDict = {}
    for dirName in dirNames:
        descriptorL = np.empty((0, 128))
        for filename in fileNames:
            img = cv.imread('the2_data/train/' + dirName + '/' + filename)
            gray = cv.cvtColor(img, cv.COLOR_BGR2GRAY)
            sift = cv.SIFT_create(edgeThreshold=11)
            kp = [cv.KeyPoint(x, y, stepSize) for y in range(0, gray.shape[0], stepSize)
                  for x in range(0, gray.shape[1], stepSize)]
            kp, des = sift.compute(gray, kp)
            if (len(kp) != 0):
                descriptorL = np.concatenate((descriptorL,des),axis=0)
        objectsAndDescriptorsDict[dirName] = descriptorL
    return objectsAndDescriptorsDict

#Saves all descriptors found by SIFT image by image to create histogram list of train images data
def getDescriptorListImagebyImageSIFT():
    dL = []
    for dirName in dirNames:
        for filename in fileNames:
            img = cv.imread('the2_data/train/' + dirName + '/' + filename)
            gray = cv.cvtColor(img, cv.COLOR_BGR2GRAY)
            sift = cv.SIFT_create()
            kp, des = sift.detectAndCompute(gray, None)
            if (len(kp) != 0):
                descriptor = np.empty((0, 128))
                descriptor = np.concatenate((descriptor, des), axis=0)
                dL.append(descriptor)
    return dL

#Saves all descriptors found by Dense-SIFT image by image to create histogram list of train images data
def getDescriptorListImagebyImageDenseSIFT(stepSize):
    dL = []
    for dirName in dirNames:
        for filename in fileNames:
            img = cv.imread('the2_data/train/' + dirName + '/' + filename)
            gray = cv.cvtColor(img, cv.COLOR_BGR2GRAY)
            sift = cv.SIFT_create()
            kp = [cv.KeyPoint(x, y, stepSize) for y in range(0, gray.shape[0], stepSize)
                  for x in range(0, gray.shape[1], stepSize)]
            kp, des = sift.compute(gray, kp)
            if (len(kp) != 0):
                descriptor = np.empty((0, 128))
                descriptor = np.concatenate((descriptor, des), axis=0)
                dL.append(descriptor)
    return dL

#gets all descriptors from objectsAndDescriptorsDict found in denseSIFTdescriptorsOfTrain() or SIFTdescriptorsOfTrain()
#to find KMEAN
def getAllDescriptors(objectsAndDescriptorsDict):
    allDescriptors = np.empty((0,128))
    for key,values in objectsAndDescriptorsDict.items():
        allDescriptors = np.concatenate((allDescriptors,values),axis=0)
    return allDescriptors

#applies Kmeans algorithm with parameters all descriptors of training images and k value
def applyKmeans(k,allDescriptors):
    kmeans = KMeans(n_clusters=k, n_init=10)
    kmeans.fit(allDescriptors)
    return kmeans

#forms histogram for a given descriptor with parameters descriptor and kmean and normalize it
def formHistogram(descriptor,kMean):
    histogram = np.zeros(len(kMean.cluster_centers_))
    cluster_result = kMean.predict(descriptor)
    for i in cluster_result:
        histogram[i] += 1.0
    return histogram / np.linalg.norm(histogram,ord=1)

#extracts all histograms in training data and gathers them in a list to be used for KNN algorithm
def getHistogramListofTrainSIFT(kMean):
    dL = getDescriptorListImagebyImageSIFT()
    histL = []
    for descriptor in dL:
        hist = formHistogram(descriptor,kMean)
        histL.append(hist)
    return histL

#extracts all histograms in training data and gathers them in a list to be used for KNN algorithm
def getHistogramListofTrainDenseSIFT(kMean,stepSize):
    dL = getDescriptorListImagebyImageDenseSIFT(stepSize)
    histL = []
    for descriptor in dL:
        hist = formHistogram(descriptor,kMean)
        histL.append(hist)
    return histL
#applies KNN algorithm between a test image's histogram and all histograms in training set to find k closest
#neighbours of it and returns list of k closest neighbours with their indexes and distances to test image's histogram
def KNN(testHist,histList,k):
    distances = []
    for index in range (0,len(histList)):
        d = np.linalg.norm(testHist - histList[index])
        tupLe = (index,d)
        distances.append(tupLe)
    distances = sorted(distances,key=lambda x: x[1])
    return distances[:k]

#computes accuracy of a given class with parameter dirName and distances list that includes all KNN neighbours of all
#images
def getAccuracy(distancesList,dirName):

    accuracy = 0
    predictions = []
    dirNames = ['apple', 'aquarium_fish', 'beetle', 'camel', 'crab', 'cup', 'elephant', 'flatfish', 'lion', 'mushroom',
                'orange', 'pear', 'road', 'skyscraper', 'woman']
    for i in range(0,len(distancesList)):
        countL = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
        for k in range(0,len(distancesList[i])):
            countL[int(distancesList[i][k][0] / 400)] += 1
        max_value = max(countL)
        max_index = countL.index(max_value)
        predictions.append(dirNames[max_index])
    for predictedName in predictions:
        if (predictedName == dirName):
            accuracy += 1
    return accuracy

def printConfusionMatrixValues(distancesList,dirName):
    predictions = []
    dirNames = ['apple', 'aquarium_fish', 'beetle', 'camel', 'crab', 'cup', 'elephant', 'flatfish', 'lion', 'mushroom',
                'orange', 'pear', 'road', 'skyscraper', 'woman']
    for i in range(0,len(distancesList)):
        countL = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
        for k in range(0,len(distancesList[i])):
            countL[int(distancesList[i][k][0] / 400)] += 1
        max_value = max(countL)
        max_index = countL.index(max_value)
        predictions.append(dirNames[max_index])
    print("Class name->",dirName)
    for classNames in dirNames:
        print(classNames,"->",end="")
        print((predictions.count(classNames) / len(predictions)) * 100)

#runs algorithm to get confusion matrix for validation set with KMEAN k = 56 and KNN k = 64
def runValidation():
    d = SIFTdescriptorsOfTrain()
    r = getAllDescriptors(d)
    kmean = applyKmeans(56,r)
    hL = getHistogramListofTrainSIFT(kmean)
    testFileNames = []

    accuracy = 0
    cannotDetect = 0
    for filename in os.listdir('the2_data/validation/apple'):
        testFileNames.append(filename)
    for dirName in dirNames:

        distancesList = []
        for filename in testFileNames:
            img = cv.imread('the2_data/validation/' + dirName + '/' + filename)
            gray= cv.cvtColor(img,cv.COLOR_BGR2GRAY)
            sift = cv.SIFT_create()
            kp, des = sift.detectAndCompute(gray,None)
            if(len(kp) != 0):

                descriptorL = np.empty((0, 128))
                descriptorL = np.concatenate((descriptorL, des), axis=0)

                h = formHistogram(descriptorL,kmean)
                distanceL = KNN(h,hL,16)
                distancesList.append(distanceL)
            else:
                cannotDetect += 1

        printConfusionMatrixValues(distancesList,dirName)

#writes output for test set
def writeOutputForTestSet():
    with open('output.txt', 'w') as file:
        dirNames = ['apple', 'aquarium_fish', 'beetle', 'camel', 'crab', 'cup', 'elephant', 'flatfish', 'lion', 'mushroom',
                    'orange', 'pear', 'road', 'skyscraper', 'woman']
        testFileNames = []
        for testFilename in os.listdir('test'):
            testFileNames.append(testFilename)
        testFileNames.sort()
        d = SIFTdescriptorsOfTrain()
        r = getAllDescriptors(d)
        kmean = applyKmeans(56, r)
        hL = getHistogramListofTrainSIFT(kmean)

        for filename in testFileNames:
            distancesList = []
            img = cv.imread('test/' + filename)
            gray = cv.cvtColor(img, cv.COLOR_BGR2GRAY)
            sift = cv.SIFT_create()
            kp, des = sift.detectAndCompute(gray, None)
            if (len(kp) != 0):

                descriptorL = np.empty((0, 128))
                descriptorL = np.concatenate((descriptorL, des), axis=0)

                h = formHistogram(descriptorL, kmean)
                distanceL = KNN(h, hL, 64)
                distancesList.append(distanceL)
            else:
                file.write(filename + ": " + "cannot detected")
                file.write("\n")
            countL = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
            for i in range(0, len(distancesList)):
                for k in range(0, len(distancesList[i])):
                    countL[int(distancesList[i][k][0] / 400)] += 1
            max_value = max(countL)
            max_index = countL.index(max_value)
            file.write(filename + ": " + dirNames[max_index])
            file.write("\n")
