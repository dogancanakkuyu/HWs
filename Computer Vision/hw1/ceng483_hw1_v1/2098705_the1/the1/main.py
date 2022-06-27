from PIL import Image
import numpy as np
file = file = open('InstanceNames.txt')
imageNameList = file.read().splitlines()
file.close()


#computing per channel histogram which takes image's nparray and chosen interval as parameters
def perChannelHistogram(image, interval):
    hRed = [0] * interval
    hGreen = [0] * interval
    hBlue = [0] * interval
    for i in range (0,image.shape[0]):
        for k in range (0, image.shape[1]):
            hRed[int(image[i][k][0] / (256 / interval))] += 1
            hGreen[int(image[i][k][1] / (256 / interval))] += 1
            hBlue[int(image[i][k][2] / (256 / interval))] += 1

    return np.array(hRed),np.array(hGreen),np.array(hBlue)

#computing 3d color histogram which takes image's nparray and chosen interval as parameters
def ColorHistogram3d(image,interval):
    ThreeDColorHist = []
    for i in range(0,interval):
        twoD = []
        for k in range(0,interval):
            oneD = []
            for j in range(0,interval):
                oneD.append(0)
            twoD.append(oneD)
        ThreeDColorHist.append(twoD)
    for i in range (0,image.shape[0]):
        for k in range (0, image.shape[1]):
            redIndex = int(image[i][k][0] / (256 / interval))
            greenIndex = int(image[i][k][1] / (256 / interval))
            blueIndex = int(image[i][k][2] / (256 / interval))
            ThreeDColorHist[redIndex][greenIndex][blueIndex] += 1
    return np.array(ThreeDColorHist)

#computing average of KL values of R,B,G values
def takeAverage(r,g,b):
    return (r + g + b) / 3.0

#KL divergence
def kl(q,s):
    epsValue = 0.00001
    return np.sum((q + epsValue) * np.log((q + epsValue) / (s + epsValue)))

#computing top accuracy using per channel histogram
def perChannelTopAccuracy(queryNo,interval):
    qRedHistList, qGreenHistList, qBlueHistList = [], [], []
    sRedHistList, sGreenHistList, sBlueHistList = [], [], []
    accurateReceived = 0
    for image in imageNameList:
        qImage = Image.open('query_' + str(queryNo) + '/' + image)
        qArr = np.asarray(qImage)
        hRed, hGreen, hBlue = perChannelHistogram(qArr, interval)
        #### l1 normalization to query histograms ####
        qRedHistList.append(hRed / (np.linalg.norm(hRed, ord=1)))
        qGreenHistList.append(hGreen / (np.linalg.norm(hGreen, ord=1)))
        qBlueHistList.append(hBlue / (np.linalg.norm(hBlue, ord=1)))

        sImage = Image.open('support_96/' + image)
        sArr = np.asarray(sImage)
        supHRed, supHGreen, supHBlue = perChannelHistogram(sArr, interval)
        #### l1 normalization to sample histograms ####
        sRedHistList.append(supHRed / (np.linalg.norm(supHRed, ord=1)))
        sGreenHistList.append(supHGreen / (np.linalg.norm(supHGreen, ord=1)))
        sBlueHistList.append(supHBlue / (np.linalg.norm(supHBlue, ord=1)))
    for qHistIndex in range (0, len(qRedHistList)):
        KLValuesList = []
        for sHistIndex in range(0, len(sRedHistList)):
            klValue = takeAverage(kl(qRedHistList[qHistIndex],sRedHistList[sHistIndex]),kl(qGreenHistList[qHistIndex],sGreenHistList[sHistIndex]),kl(qBlueHistList[qHistIndex],sBlueHistList[sHistIndex]))
            KLValuesList.append(klValue)
        minKlValue = min(KLValuesList)
        minValueIndex = KLValuesList.index(minKlValue)
        if (minValueIndex == qHistIndex):
            accurateReceived += 1

    topAccuracy = accurateReceived / len(imageNameList)
    return topAccuracy

#computing top accuracy using 3d color histogram
def colorHistogram3dTopAccuracy(querySetNo,interval):
    query3dHistList, sample3dHistList = [], [] #Histogram lists that holds queries and samples histograms
    accurateReceived = 0
    for image in imageNameList:
        qImage = Image.open('query_' + str(querySetNo) + '/' + image)
        qArr = np.asarray(qImage)
        qHistogram = ColorHistogram3d(qArr,interval)
        #### l1 normalization to query histogram ####
        query3dHistList.append(qHistogram.flatten() / (np.linalg.norm(qHistogram.flatten(),ord=1)))

        sImage = Image.open('support_96/' + image)
        sArr = np.asarray(sImage)
        sHistogram = ColorHistogram3d(sArr,interval)
        #### l1 normalization to sample histogram ####
        sample3dHistList.append(sHistogram.flatten() / (np.linalg.norm(sHistogram.flatten(),ord=1)))
    for qHistIndex in range(0, len(query3dHistList)):
        KLValuesList = []
        for sHistIndex in range(0, len(sample3dHistList)):
            klValue = kl(query3dHistList[qHistIndex],sample3dHistList[sHistIndex])
            KLValuesList.append(klValue)
        minKlValue = min(KLValuesList)
        minValueIndex = KLValuesList.index(minKlValue)
        if (minValueIndex == qHistIndex):
            accurateReceived += 1
    topAccuracy = accurateReceived / len(imageNameList)
    return topAccuracy

#computing top accuracy using grid spatial 3d color histogram. It takes query set number
#grid dimension and chosen interval as parameters. For example query1, 48x48 grid and 16 as interval
#it takes gridSpatialTopAccuracy3dColor(1,48,16)
def gridSpatialTopAccuracy3dColor(querySetNo,gridDimension,interval):
    queryGridHistList, sampleGridHistList = [], []
    accurateReceived = 0
    for i in range(0, len(imageNameList)):
        queryGridHistList.append([])
        sampleGridHistList.append([])
    gridListIndex = 0
    for image in imageNameList:
        qImage = Image.open('query_' + str(querySetNo) + '/' + image)
        qArr = np.asarray(qImage)
        sImage = Image.open('support_96/' + image)
        sArr = np.asarray(sImage)

        #forming grids according to given grid dimension
        qGridArr = qArr.reshape(96 // gridDimension, gridDimension, 96 // gridDimension, gridDimension, 3)
        qGridArr = qGridArr.swapaxes(1, 2)
        sGridArr = sArr.reshape(96 // gridDimension, gridDimension, 96 // gridDimension, gridDimension, 3)
        sGridArr = sGridArr.swapaxes(1, 2)

        for i in range (0, len(qGridArr)):
            for k in range (0, len(qGridArr)):
                ### l1 normalization to histograms ###
                normedQueryHist = ColorHistogram3d(qGridArr[i][k],interval).flatten() / (np.linalg.norm(ColorHistogram3d(qGridArr[i][k],interval).flatten(),ord=1))
                normedSampleHist = ColorHistogram3d(sGridArr[i][k],interval).flatten() / (np.linalg.norm(ColorHistogram3d(sGridArr[i][k],interval).flatten(),ord=1))

                queryGridHistList[gridListIndex].append(normedQueryHist)
                sampleGridHistList[gridListIndex].append(normedSampleHist)
        gridListIndex += 1
    for i1 in range (0, len(queryGridHistList)):
        KLValuesList = []
        for i2 in range (0, len(sampleGridHistList)):
            KlValue = 0
            for hist in range (0, len(queryGridHistList[i])):
                KlValue += kl(queryGridHistList[i1][hist],sampleGridHistList[i2][hist])
            KLValuesList.append(KlValue)
        minKlValue = min(KLValuesList)
        minValueIndex = KLValuesList.index(minKlValue)
        if (minValueIndex == i1):
            accurateReceived += 1
    topAccuracy = accurateReceived / len(imageNameList)
    return topAccuracy



#computing top accuracy using grid spatial per channel histogram. It takes query set number
#grid dimension and chosen interval as parameters. For example query1, 48x48 grid and 16 as interval
#it takes gridSpatialTopAccuracyPerChannel(1,48,16)
def gridSpatialTopAccuracyPerChannel(querySetNo,gridDimension,interval):
    queryRHistL, queryGHistL, queryBHistL = [], [], []
    sampleRHistL, sampleGHistL, sampleBHistL = [], [], []
    accurateReceived = 0
    for i in range(0, len(imageNameList)):
        queryRHistL.append([])
        queryGHistL.append([])
        queryBHistL.append([])
        sampleRHistL.append([])
        sampleGHistL.append([])
        sampleBHistL.append([])
    gridListIndex = 0
    for image in imageNameList:
        qImage = Image.open('query_' + str(querySetNo) + '/' + image)
        qArr = np.asarray(qImage)
        sImage = Image.open('support_96/' + image)
        sArr = np.asarray(sImage)

        # forming grids according to given grid dimension
        qGridArr = qArr.reshape(96 // gridDimension, gridDimension, 96 // gridDimension, gridDimension, 3)
        qGridArr = qGridArr.swapaxes(1, 2)
        sGridArr = sArr.reshape(96 // gridDimension, gridDimension, 96 // gridDimension, gridDimension, 3)
        sGridArr = sGridArr.swapaxes(1, 2)
        for i in range (0, len(qGridArr)):
            for k in range (0, len(qGridArr)):
                hRed, hGreen, hBlue = perChannelHistogram(qGridArr[i][k], interval)
                ## l1 normalization to query histograms ##
                queryRHistL[gridListIndex].append((hRed / (np.linalg.norm(hRed, ord=1))))
                queryGHistL[gridListIndex].append((hGreen / (np.linalg.norm(hGreen, ord=1))))
                queryBHistL[gridListIndex].append((hBlue / (np.linalg.norm(hBlue, ord=1))))

                sRed, sGreen, sBlue = perChannelHistogram(sGridArr[i][k], interval)
                #### l1 normalization to sample histograms ####
                sampleRHistL[gridListIndex].append((sRed / (np.linalg.norm(sRed, ord=1))))
                sampleGHistL[gridListIndex].append((sGreen / (np.linalg.norm(sGreen, ord=1))))
                sampleBHistL[gridListIndex].append((sBlue / (np.linalg.norm(sBlue, ord=1))))
        gridListIndex += 1
    for i1 in range (0, len(queryRHistL)):
        KLValuesList = []
        for i2 in range (0, len(sampleRHistL)):
            KlValue = 0
            for hist in range (0, len(queryRHistL[i])):
                rKlValue = kl(queryRHistL[i1][hist],sampleRHistL[i2][hist])
                gKlValue = kl(queryGHistL[i1][hist],sampleGHistL[i2][hist])
                bKlValue = kl(queryBHistL[i1][hist],sampleBHistL[i2][hist])
                KlValue += takeAverage(rKlValue,gKlValue,bKlValue)
            averageOfHistograms = KlValue / ((96 / gridDimension) ** 2 )
            KLValuesList.append(averageOfHistograms)
        minKlValue = min(KLValuesList)
        minValueIndex = KLValuesList.index(minKlValue)
        if (minValueIndex == i1):
            accurateReceived += 1
    topAccuracy = accurateReceived / len(imageNameList)
    return topAccuracy



