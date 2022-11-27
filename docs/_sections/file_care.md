---
layout: post
title: File Care
---

After transferring, analyzing, and (optionally) packaging DV streams, they will begin their new life as files. 

## Community-established practices

Digital preservation is an ever-evolving field, and decisions around good digital stewardship will differ between institutions, their collecting policies, their financial situations, and a myriad of other factors. Long-term preservation of hundreds of hours of DV camera footage for a small non-profit, DV documenting human rights concerns intended for use in a court case, and DV that are part of an artwork held by a national art museum will all require different levels, strategies, and approaches to preservation. While DVRescue provides the highest level of care to the transfer and analysis process, the longer term preservation actions and storage options will vary. All of this to say: the best digital preservation practices will be determined by you and your organization's preservation policies, and will likely be different from other preservation situations.

In many ways, DV can be treated like any other bitstream data destined for preservation. If your institution already has established practices around handling digital objects for preservation and access, DV should fit within those frameworks as much as possible.

If getting started with digital preservation for the first time, it is worth considering the [NDSA Levels of Digital Preservation](https://ndsa.org/publications/levels-of-digital-preservation/) and the [Digital Preservation Coalition's Digital Preservation Handbook](https://www.dpconline.org/handbook). These resources can get you started with some of the foundational elements of long-term file care, such as integrity checking, metadata guidelines, and storage requirements.

## Creating archival access derivatives

If you are currently using a digital preservation or archival access system that will create access derivatives for you, such as files suitable for giving to clients/patrons or for playback on the web... great! Allow that system to do the work for you.

There are situations when will need to create your own copies of access files.

MIPoPS uses and recommends the following settings:  

- Picture Settings
	- Size
		- Anamorphic = Strict
		- Cropping = Custom (if Automatic doesn’t crop out the head switching noise or black bars)
	- Filter Settings
		- Decomb = Default
		- Deinterlace = Off
- Video Settings (main app window)
	- Constant Quality = On
	- Web optimized = On

[Handbrake](https://handbrake.fr/) is an open source video transcoding tool available for all major operating systems. This is a convenient way to create copies of any files, and it works with DV, too.

From DV, you can use Handbrake to create an MP4 video file with h264 video encoding and AAC audio encoding. This will create an access file that is suitable for web access, sharing on social media, adding to a content management system, or for file-sharing. This file will be smaller, no longer contain some properties of DV for future analysis, and the quality will not be as high as the original.

You can download the <a href="{{ site.baseurl }}/_data/MIPoPS_Handbrake_Access.json">MIPoPS Handbrake Access JSON data</a> and add it to your Handbrake, to use the same recommended settings without having to configure them yourself.

## Creating presentation derivatives

DV can be presented directly in most video players. There may be a situation in which a .dv file is not accepted, and you can use the Packaging component of DVRescue to wrap the DV with an Apple Quicktime (MOV) wrapper. This will retain the original elements of the DV stream while being able to offer a more familiar file extension.

