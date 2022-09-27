#include "machelpers.h"

#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>

@interface PanelDelegate: NSObject <NSOpenSavePanelDelegate>
@property (retain,nonatomic) NSURL *dirURL;

- (id)initWithDirectoryURL:(NSURL *)URL;
@end

@implementation PanelDelegate
- (id)initWithDirectoryURL:(NSURL*)url
{
    self = [super init];

    if (self)
        _dirURL = url;

    return self;
}
@end

QString sandboxQueryRWPermissionForPath(const QString& dirPath, const QString& message)
{
    QString ret = QString();

    NSURL *dirURL = [NSURL fileURLWithPath:dirPath.toNSString()];
    PanelDelegate *delegate = [[PanelDelegate alloc] initWithDirectoryURL:dirURL];

    NSOpenPanel *openPanel = [NSOpenPanel openPanel];
    [openPanel setCanChooseFiles:NO];
    [openPanel setCanCreateDirectories:NO];
    [openPanel setCanChooseDirectories:YES];
    [openPanel setAllowsMultipleSelection:NO];
    [openPanel setDirectoryURL:dirURL];
    [openPanel setMessage:message.toNSString()];
    [openPanel setDelegate:delegate];

    if ([openPanel runModal] == NSModalResponseOK)
        ret = QString::fromNSString([openPanel.URL path]);

    return ret;
}
