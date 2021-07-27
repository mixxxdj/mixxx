<?xml version="1.0" encoding="UTF-8"?>
<!--
This XSLT stylesheet is taken from the jenkins-ctest-plugin by Version One,
Inc. and Ryan Pavlik <abiryan@ryand.net> and is subject to the terms of the
MIT License.

It was taken from this GitHub repository:
    https://github.com/rpavlik/jenkins-ctest-plugin

Includes modifications by Jan Holthuis to add support for skipped/errored
tests.
-->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
	<xsl:output method="xml" indent="yes" />
	<xsl:template match="/">
		<testsuites>
			<xsl:variable name="buildName" select="//Site/@BuildName"/>
			<xsl:variable name="numberOfTests" select="count(//Site/Testing/Test)"/>
			<xsl:variable name="numberOfFailures" select="count(//Site/Testing/Test[@Status!='passed'])" />
			<testsuite name="CTest"
				tests="{$numberOfTests}" time="0"
				failures="{$numberOfFailures}"  errors="0"
				skipped="0">
			<xsl:for-each select="//Site/Testing/Test">
					<xsl:variable name="testName" select="translate(Name, '-', '_')"/>
					<xsl:variable name="duration" select="Results/NamedMeasurement[@name='Execution Time']/Value"/>
					<xsl:variable name="status" select="@Status"/>
					<xsl:variable name="output" select="Results/Measurement/Value"/>
					<xsl:variable name="className" select="translate(Path, '/.', '.')"/>
					<testcase classname="projectroot{$className}"
						name="{$testName}"
						time="{$duration}">
						<xsl:if test="@Status='failed'">
							<failure>
								<xsl:value-of select="$output" />
							</failure>
						</xsl:if>
						<xsl:if test="@Status='errored'">
							<error>
								<xsl:value-of select="$output" />
							</error>
						</xsl:if>
						<xsl:if test="@Status='notrun'">
							<skipped />
						</xsl:if>
						<system-out>
							<xsl:value-of select="$output" />
						</system-out>
					</testcase>
				</xsl:for-each>
			</testsuite>
		</testsuites>
	</xsl:template>
</xsl:stylesheet>
