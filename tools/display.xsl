<?xml version="1.0"?>
<!-- WIP -->
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:dv="https://mediaarea.net/dvrescue" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:x="http://www.w3.org/1999/xhtml" version="1.0" extension-element-prefixes="xsi" exclude-result-prefixes="dv x" >
<xsl:output encoding="UTF-8" method="html" version="1.0" indent="yes" doctype-system="about:legacy-compat"/>
<xsl:template match="/dv:dvrescue">
  <html>
    <head>
      <style type="text/css">
        body {
          font-family: "PT Sans", Helvetica, Arial, sans-serif;
          max-width: 1200px;
          display: grid;
          justify-content: center;
          justify-items: center;
          overflow-wrap: anywhere;
        }
        h1 {
          font-weight: 800;
          font-style: italic;
        }
        img {
          width: 200px;
          height: inherit;
        }
        td {
          padding: 2em;
        }
        dt {
          font-weight: 800;
          font-style: italic;
        }
        .frames {
          display: flex;
          flex-wrap: wrap;
          justify-content: center;
        }
      </style>
    </head>
    <body>
      <xsl:for-each select="dv:media">
        <header>
          <h1>DVRescue Report</h1>
          <h2><xsl:value-of select="@ref"/></h2>
        </header>
        <xsl:for-each select="dv:frames">
          <section class="metadata">
            <!-- These need to only appear if present -->
            <table>
              <thead><tr><th colspan="2">Frames metadata</th></tr></thead>
              <tbody>
                <tr>
                  <td>
                    <dl>
                      <dt>Count</dt><dd><xsl:value-of select="@count"/></dd>
                      <dt>Start timestamp</dt><dd><xsl:value-of select="@pts"/></dd>
                      <dt>End timestamp</dt><dd><xsl:value-of select="@end_pts"/></dd>
                      <dt>Size</dt><dd><xsl:value-of select="@size"/></dd>
                      <dt>Video rate</dt><dd><xsl:value-of select="@video_rate"/></dd>
                    </dl>
                  </td>
                  <td>
                    <dl>
                      <dt>Aspect ratio</dt><dd><xsl:value-of select="@aspect_ratio"/></dd>
                      <dt>Chroma subsampling</dt><dd><xsl:value-of select="@chroma_subsampling"/></dd>
                      <dt>Audio rate</dt><dd><xsl:value-of select="@audio_rate"/></dd>
                      <dt>Channels</dt><dd><xsl:value-of select="@channels"/></dd>
                    </dl>
                  </td>
                </tr>
              </tbody>
            </table>
          </section>
          <section class="frames">
            <!-- Ins/outs should either have frames cut or be hidden from this view -->
            <xsl:for-each select="dv:frame">
              <div>
                <img>
                  <xsl:attribute name="src"><xsl:value-of select="@tc"/>.jpg</xsl:attribute>
                </img>
              <p>TC <xsl:value-of select="@tc"/></p>
              </div>
            </xsl:for-each>
          </section>
        </xsl:for-each>
      </xsl:for-each>
      <footer><xsl:value-of select="dv:creator/dv:program"/> v.<xsl:value-of select="dv:creator/dv:version"/></footer>
    </body>
  </html>
  </xsl:template>
</xsl:stylesheet>
