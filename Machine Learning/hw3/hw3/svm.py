import numpy as np
from sklearn.svm import SVC
import matplotlib.pyplot as plt
def draw_svm(clf, x, y, x1_min, x1_max, x2_min, x2_max, target=None):
    """
    Draws the decision boundary of an svm.
    :param clf: sklearn.svm.SVC classifier
    :param x: data Nx2
    :param y: label N
    :param x1_min: minimum value of the x-axis of the plot
    :param x1_max: maximum value of the x-axis of the plot
    :param x2_min: minimum value of the y-axis of the plot
    :param x2_max: maximum value of the y-axis of the plot
    :param target: if target is set to path, the plot is saved to that path
    :return: None
    """
    y = y.astype(bool)
    xx, yy = np.meshgrid(np.linspace(x1_min, x1_max, 500),
                         np.linspace(x2_min, x2_max, 500))
    z = clf.decision_function(np.c_[xx.ravel(), yy.ravel()])
    z = z.reshape(xx.shape)
    disc_z = z > 0
    plt.clf()
    plt.imshow(disc_z, interpolation='nearest',
               extent=(xx.min(), xx.max(), yy.min(), yy.max()), aspect='auto',
               origin='lower', cmap=plt.cm.RdBu, alpha=.3)
    plt.contour(xx, yy, z, levels=[-1, 1], linewidths=2,
                linestyles='dashed', colors=['red', 'blue'], alpha=0.5)
    plt.contour(xx, yy, z, levels=[0], linewidths=2,
                linestyles='solid', colors='black', alpha=0.5)
    positives = x[y == 1]
    negatives = x[y == 0]
    plt.scatter(positives[:, 0], positives[:, 1], s=50, marker='o', color="none", edgecolor="black")
    plt.scatter(negatives[:, 0], negatives[:, 1], s=50, marker='s', color="none", edgecolor="black")
    sv_label = y[clf.support_]
    positive_sv = x[clf.support_][sv_label]
    negative_sv = x[clf.support_][~sv_label]
    plt.scatter(positive_sv[:, 0], positive_sv[:, 1], s=50, marker='o', color="white", edgecolor="black")
    plt.scatter(negative_sv[:, 0], negative_sv[:, 1], s=50, marker='s', color="white", edgecolor="black")
    plt.xlim(x1_min, x1_max)
    plt.ylim(x2_min, x2_max)
    plt.gca().set_aspect('equal', adjustable='box')
    if target is None:
        plt.show()
    else:
        plt.savefig(target)

def task1():
    trainSet = np.load('svm/task1/train_set.npy')
    trainLabels = np.load('svm/task1/train_labels.npy')
    plt.scatter(trainSet[trainLabels == 0,0], trainSet[trainLabels == 0,1])
    plt.scatter(trainSet[trainLabels == 1, 0], trainSet[trainLabels == 1, 1])
    plt.show()

    clf = SVC(kernel='linear', C=100)
    clf = clf.fit(trainSet, trainLabels)
    draw_svm(clf,trainSet,trainLabels,trainSet[:,0].min(),trainSet[:,0].max(),trainSet[:,1].min(),trainSet[:,1].max())

def task2():
    trainSet = np.load('svm/task2/train_set.npy')
    trainLabels = np.load('svm/task2/train_labels.npy')
    plt.scatter(trainSet[trainLabels == 0, 0], trainSet[trainLabels == 0, 1])
    plt.scatter(trainSet[trainLabels == 1, 0], trainSet[trainLabels == 1, 1])
    plt.show()
    clfLinear = SVC(kernel='linear', C=1)
    clfLinear = clfLinear.fit(trainSet,trainLabels)
    draw_svm(clfLinear,trainSet,trainLabels,trainSet[:,0].min(),trainSet[:,0].max(),trainSet[:,1].min(),trainSet[:,1].max())
    clfrbf = SVC(kernel='rbf',C=1)
    clfrbf = clfrbf.fit(trainSet,trainLabels)
    draw_svm(clfrbf,trainSet,trainLabels,trainSet[:,0].min(),trainSet[:,0].max(),trainSet[:,1].min(),trainSet[:,1].max())
    clfPoly = SVC(kernel='poly', C=1)
    clfPoly = clfPoly.fit(trainSet,trainLabels)
    draw_svm(clfPoly,trainSet,trainLabels,trainSet[:,0].min(),trainSet[:,0].max(),trainSet[:,1].min(),trainSet[:,1].max())
    clfSigmoid = SVC(kernel='sigmoid',C=1)
    clfSigmoid = clfSigmoid.fit(trainSet,trainLabels)
    draw_svm(clfSigmoid,trainSet,trainLabels,trainSet[:,0].min(),trainSet[:,0].max(),trainSet[:,1].min(),trainSet[:,1].max())

def task3():
    trainSet = np.load('svm/task3/train_set.npy')
    trainLabels = np.load('svm/task3/train_labels.npy')
    testSet = np.load('svm/task3/test_set.npy')
    testLabels = np.load('svm/task3/test_labels.npy')
    validationSet = trainSet[800:1000]
    validationLabels = trainLabels[800:1000]
    trainSetPartial = trainSet[0:800]
    trainLabelsPartial = trainLabels[0:800]

    #reshape arrays
    x,y,z = trainSetPartial.shape
    trainSetPartial = trainSetPartial.reshape((x,y * z))
    x1,y1,z1 = validationSet.shape
    validationSet = validationSet.reshape((x1,y1 * z1))
    x2,y2,z2 = trainSet.shape
    trainSet = trainSet.reshape((x2,y2 * z2))
    x3,y3,z3 = testSet.shape
    testSet = testSet.reshape((x3,y3 * z3))

    #classifier configuration and validation set accuracy
    classifier = SVC(kernel='rbf',C=10,gamma='scale') #tuning configuration
    classifier = classifier.fit(trainSetPartial,trainLabelsPartial)
    predictedLabels = classifier.predict(validationSet)
    accuracyValid = 0
    for i in range(0,len(predictedLabels)):
        if(predictedLabels[i] == validationLabels[i]):
            accuracyValid += 1
    accuracyValid /= (len(predictedLabels))


    #computing test accuracy using whole train set

    classifierTest = SVC(kernel='rbf',C=10,gamma='scale') #best configuration
    classifierTest = classifierTest.fit(trainSet,trainLabels)
    predictedLabelsForTest = classifierTest.predict(testSet)
    accuracyTest = 0
    for k in range(0,len(predictedLabelsForTest)):
        if(predictedLabelsForTest[i] == testLabels[i]):
            accuracyTest += 1
    accuracyTest /= len(predictedLabelsForTest)
    print(accuracyTest)

def task4():
    trainSet = np.load('svm/task4/train_set.npy')
    trainLabels = np.load('svm/task4/train_labels.npy')
    testSet = np.load('svm/task4/test_set.npy')
    testLabels = np.load('svm/task4/test_labels.npy')

    #reshape arrays
    x,y,z = trainSet.shape
    trainSet = trainSet.reshape((x,y * z))
    x1,y1,z1 = testSet.shape
    testSet = testSet.reshape((x1,y1 * z1))

    #classifier
    clf = SVC(kernel='rbf',C=1)
    clf = clf.fit(trainSet,trainLabels)
    predictedLabels = clf.predict(testSet)
    TP = 0
    FP = 0
    TN = 0
    FN = 0
    for i in range(0,len(predictedLabels)):
        if(predictedLabels[i] == 0 and testLabels[i] == 1):
            FN += 1
        elif(predictedLabels[i] == 1 and testLabels[i] == 1):
            TP += 1
        elif(predictedLabels[i] == 1 and testLabels[i] == 0):
            FP += 1
        elif(predictedLabels[i] == 0 and testLabels[i] == 0):
            TN += 1
    accuracy = (TP + TN) / (TP + TN + FP + FN)


    
    #oversampling
    noOfMinority = 0
    noOfMajority = 0
    for i in trainLabels:
        if(i == 0):
            noOfMinority += 1
        else:
            noOfMajority += 1
    oversampledTrainSet = trainSet
    oversampledTrainLabels = trainLabels
    minoritySet = []

    for i in range(0,len(trainLabels)):
        if(trainLabels[i] == 0):
            minoritySet.append(trainSet[i])
    index = 0
    while(noOfMajority != noOfMinority):
        if(index == len(minoritySet)):
            index = 0
        oversampledTrainSet = np.vstack((oversampledTrainSet,minoritySet[index]))
        oversampledTrainLabels = np.append(oversampledTrainLabels,0)
        index += 1
        noOfMinority += 1

    clfOverSample = SVC(kernel='rbf',C=1)
    clfOverSample = clfOverSample.fit(oversampledTrainSet,oversampledTrainLabels)
    predictedOverSample = clfOverSample.predict(testSet)
    TPOverS = 0
    FPOverS = 0
    TNOverS = 0
    FNOverS = 0
    for i in range(0,len(predictedOverSample)):
        if (predictedOverSample[i] == 0 and testLabels[i] == 1):
            FNOverS += 1
        elif (predictedOverSample[i] == 1 and testLabels[i] == 1):
            TPOverS += 1
        elif (predictedOverSample[i] == 1 and testLabels[i] == 0):
            FPOverS += 1
        elif (predictedOverSample[i] == 0 and testLabels[i] == 0):
            TNOverS += 1
    accuracyOverS = (TPOverS + TNOverS) / (TPOverS + TNOverS + FPOverS + FNOverS)



    #undersampling
    noOfMinority = 0
    noOfMajority = 0
    for i in trainLabels:
        if (i == 0):
            noOfMinority += 1
        else:
            noOfMajority += 1
    underSampledTrainSet = np.empty(0)
    underSampledTrainLabels = []
    addedMajority = 0
    for i in range(0,len(trainSet)):
        if(len(underSampledTrainSet) == 0):
            if(trainLabels[i] == 0):
                underSampledTrainSet = np.concatenate((underSampledTrainSet, trainSet[i]), axis=0)
                underSampledTrainLabels.append(0)
            elif(trainLabels[i] == 1):
                underSampledTrainSet = np.concatenate((underSampledTrainSet, trainSet[0]), axis=0)
                underSampledTrainLabels.append(1)
                addedMajority += 1
        else:
            if (trainLabels[i] == 0):
                underSampledTrainSet = np.vstack((underSampledTrainSet,trainSet[i]))
                underSampledTrainLabels.append(0)
            elif (trainLabels[i] == 1):
                if (addedMajority < 50):
                    underSampledTrainSet = np.vstack((underSampledTrainSet, trainSet[i]))
                    underSampledTrainLabels.append(1)
                    addedMajority += 1

    underSampledTrainLabels = np.array(underSampledTrainLabels)
    clfUnderSample = SVC(kernel='rbf',C=1)
    clfUnderSample = clfUnderSample.fit(underSampledTrainSet,underSampledTrainLabels)
    predictedUnderSample = clfUnderSample.predict(testSet)
    TPUnderS = 0
    FPUnderS = 0
    TNUnderS = 0
    FNUnderS = 0
    for i in range(0, len(predictedUnderSample)):
        if (predictedUnderSample[i] == 0 and testLabels[i] == 1):
            FNUnderS += 1
        elif (predictedUnderSample[i] == 1 and testLabels[i] == 1):
            TPUnderS += 1
        elif (predictedUnderSample[i] == 1 and testLabels[i] == 0):
            FPUnderS += 1
        elif (predictedUnderSample[i] == 0 and testLabels[i] == 0):
            TNUnderS += 1
    accuracyUnderS = (TPUnderS + TNUnderS) / (TPUnderS + TNUnderS + FPUnderS + FNUnderS)

    #class_weight balanced

    clfBalanced = SVC(kernel='rbf',C=1,class_weight='balanced')
    clfBalanced = clfBalanced.fit(trainSet,trainLabels)
    predictedBalanced = clfBalanced.predict(testSet)
    TPBalanced = 0
    FPBalanced = 0
    TNBalanced = 0
    FNBalanced = 0
    for i in range(0,len(predictedBalanced)):
        if (predictedBalanced[i] == 0 and testLabels[i] == 1):
            FNBalanced += 1
        elif (predictedBalanced[i] == 1 and testLabels[i] == 1):
            TPBalanced += 1
        elif (predictedBalanced[i] == 1 and testLabels[i] == 0):
            FPBalanced += 1
        elif (predictedBalanced[i] == 0 and testLabels[i] == 0):
            TNBalanced += 1
    accuracyBalanced = (TPBalanced + TNBalanced) / (TPBalanced + TNBalanced + FPBalanced + FNBalanced)
